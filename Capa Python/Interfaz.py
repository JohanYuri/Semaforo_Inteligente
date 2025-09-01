import tkinter as tk
from tkinter import font
import requests
import time

# --- CONFIGURACIÓN ---
esp32_ip = "192.168.1.82"  # IP del ESP32

# --- VENTANA PRINCIPAL ---
window = tk.Tk()
window.title("🚦 Control del Semáforo")
window.geometry("1000x700")  # Tamaño fijo más grande
window.resizable(False, False)
window.config(bg="#1e1e1e")  # Fondo oscuro

# --- ESTILOS ---
TITLE_FONT = ("Segoe UI", 14, "bold")
LABEL_FONT = ("Segoe UI", 11, "bold")
BUTTON_FONT = ("Segoe UI", 10, "bold")
SLIDER_COLOR = "#3e3e3e"
BTN_ON_COLOR = "#27ae60"
BTN_OFF_COLOR = "#c0392b"
BTN_ROUTINE_COLOR = "#2980b9"
TEXT_COLOR = "#ecf0f1"

# --- Variables globales para el parpadeo ---
blink_state = False
blink_last_time = 0

# --- FUNCIONES DE COMUNICACIÓN ---
def send_request(path):
    try:
        url = f"http://{esp32_ip}/{path}"
        response = requests.get(url, timeout=2)
        print(f"Enviado: {url} -> Respuesta: {response.status_code}, {response.text}")
        return response
    except requests.exceptions.RequestException as e:
        print(f"Error de conexión: {e}")
        return None

def get_led_status(color):
    """Obtiene el estado de un LED específico"""
    try:
        response = send_request(f"led/{color}/status")
        if response and response.status_code == 200:
            return response.text.strip() == "Encendido"
        return False
    except:
        return False
    

def update_traffic_light_ui(red_on, yellow_on, green_on, blinking_green=False):
    """Actualiza la interfaz gráfica del semáforo con múltiples estados."""
    global blink_state, blink_last_time

    canvas.itemconfig(red_light, fill="#e74c3c" if red_on else "#2c2c2c")
    canvas.itemconfig(yellow_light, fill="#f1c40f" if yellow_on else "#2c2c2c")

    # Manejo especial para el verde parpadeante
    if blinking_green:
        current_time = time.time() * 1000  # Tiempo en ms
        if current_time - blink_last_time >= 500:  # 500ms para parpadeo
            blink_last_time = current_time
            blink_state = not blink_state
        canvas.itemconfig(green_light, fill="#2ecc71" if blink_state else "#2c2c2c")
    else:
        canvas.itemconfig(green_light, fill="#2ecc71" if green_on else "#2c2c2c")


# # --- FUNCION PARA OBTENER EL ESTADO DEL ESP32 ---
def fetch_status():
    """Consulta el estado del semáforo en el ESP32 y actualiza la UI."""
    try:
        url = f"http://{esp32_ip}/status"
        response = requests.get(url, timeout=1)
        if response.status_code == 200:
            status = response.text.strip()

            if status == "verde":
                update_traffic_light_ui(False, False, True)
            elif status == "amarillo":
                update_traffic_light_ui(False, True, False)
            elif status == "rojo":
                update_traffic_light_ui(True, False, False)
            elif status == "verde_parpadeando":
                update_traffic_light_ui(False, False, False, True)
            elif status == "todos":
                update_traffic_light_ui(True, True, True)
            elif status == "ninguno":
                update_traffic_light_ui(False, False, False)
            else:
                # Consultamos cada LED individualmente
                red_on = get_led_status("rojo")
                yellow_on = get_led_status("amarillo")
                green_on = get_led_status("verde")
                update_traffic_light_ui(red_on, yellow_on, green_on)

    except requests.exceptions.RequestException as e:
        print(f"Error al obtener estado: {e}")

    # Programar siguiente actualización
    window.after(100, fetch_status)

# --- FUNCIONES DE CONTROL ---
def control_manual(color, state):
    send_request(f"led/{color}/{state}")

def start_routine():
    t_verde = int(scale_verde.get() * 1000)
    t_amarillo = int(scale_amarillo.get() * 1000)
    t_rojo = int(scale_rojo.get() * 1000)
    path = f"rutina/start?verde={t_verde}&amarillo={t_amarillo}&rojo={t_rojo}"
    send_request(path)

def stop_routine():
    send_request("rutina/stop")

# --- SEMÁFORO ---
traffic_light_frame = tk.Frame(window, bg="#1e1e1e", width=200, height=400)
traffic_light_frame.place(x=100, y=120)

canvas = tk.Canvas(
    traffic_light_frame,
    bg="#111111",
    width=180,
    height=400,
    highlightthickness=0,
    bd=0
)
canvas.pack(pady=15)

red_light = canvas.create_oval(30, 30, 150, 130, fill="#2c2c2c", outline="#444", width=3)
yellow_light = canvas.create_oval(30, 150, 150, 250, fill="#2c2c2c", outline="#444", width=3)
green_light = canvas.create_oval(30, 270, 150, 370, fill="#2c2c2c", outline="#444", width=3)

# --- PANEL DE CONTROLES ---
controls_frame = tk.Frame(window, bg="#1e1e1e")
controls_frame.place(x=350, y=80)

# Manual Frame
manual_frame = tk.LabelFrame(
    controls_frame,
    text="🔧 Control por Lámpara",
    padx=10,
    pady=10,
    font=LABEL_FONT,
    bg="#2c2c2c",
    fg=TEXT_COLOR,
    labelanchor="n"
)
manual_frame.grid(row=0, column=0, padx=10, pady=10, sticky="ew")

def make_button(frame, text, color, cmd):
    return tk.Button(
        frame,
        text=text,
        font=BUTTON_FONT,
        bg=color,
        fg="white",
        activebackground="#555",
        width=10,
        relief="flat",
        command=cmd
    )

# Botones manuales
tk.Label(manual_frame, text="Verde:", bg="#2c2c2c", fg=TEXT_COLOR, font=LABEL_FONT).grid(row=0, column=0, padx=5, pady=5)
make_button(manual_frame, "Encender", BTN_ON_COLOR, lambda: control_manual("verde", "on")).grid(row=0, column=1, padx=5)
make_button(manual_frame, "Apagar", BTN_OFF_COLOR, lambda: control_manual("verde", "off")).grid(row=0, column=2, padx=5)

tk.Label(manual_frame, text="Amarillo:", bg="#2c2c2c", fg=TEXT_COLOR, font=LABEL_FONT).grid(row=1, column=0, padx=5, pady=5)
make_button(manual_frame, "Encender", BTN_ON_COLOR, lambda: control_manual("amarillo", "on")).grid(row=1, column=1, padx=5)
make_button(manual_frame, "Apagar", BTN_OFF_COLOR, lambda: control_manual("amarillo", "off")).grid(row=1, column=2, padx=5)

tk.Label(manual_frame, text="Rojo:", bg="#2c2c2c", fg=TEXT_COLOR, font=LABEL_FONT).grid(row=2, column=0, padx=5, pady=5)
make_button(manual_frame, "Encender", BTN_ON_COLOR, lambda: control_manual("rojo", "on")).grid(row=2, column=1, padx=5)
make_button(manual_frame, "Apagar", BTN_OFF_COLOR, lambda: control_manual("rojo", "off")).grid(row=2, column=2, padx=5)

# Todos
tk.Label(manual_frame, text="Todos:", bg="#2c2c2c", fg=TEXT_COLOR, font=LABEL_FONT).grid(row=3, column=0, padx=5, pady=5)
make_button(manual_frame, "Encender", "#9b59b6", lambda: [control_manual("verde", "on"), control_manual("amarillo", "on"), control_manual("rojo", "on")]).grid(row=3, column=1, padx=5)
make_button(manual_frame, "Apagar", BTN_OFF_COLOR, lambda: [control_manual("verde", "off"), control_manual("amarillo", "off"), control_manual("rojo", "off")]).grid(row=3, column=2, padx=5)

# Rutina
routine_frame = tk.LabelFrame(
    controls_frame,
    text="⏱️ Control de Rutina",
    padx=10,
    pady=10,
    font=LABEL_FONT,
    bg="#2c2c2c",
    fg=TEXT_COLOR,
    labelanchor="n"
)
routine_frame.grid(row=1, column=0, padx=10, pady=10, sticky="ew")

make_button(routine_frame, "Iniciar Rutina", BTN_ROUTINE_COLOR, start_routine).pack(pady=5)
make_button(routine_frame, "Parar Rutina", BTN_OFF_COLOR, stop_routine).pack(pady=5)

# Sliders
time_frame = tk.LabelFrame(
    controls_frame,
    text="⚙️ Sintonizar Tiempos (segundos)",
    padx=10,
    pady=10,
    font=LABEL_FONT,
    bg="#2c2c2c",
    fg=TEXT_COLOR,
    labelanchor="n"
)
time_frame.grid(row=2, column=0, padx=10, pady=10, sticky="ew")

def make_slider(frame, text, from_, to, init):
    tk.Label(frame, text=text, bg="#2c2c2c", fg=TEXT_COLOR, font=LABEL_FONT).pack()
    slider = tk.Scale(
        frame,
        from_=from_,
        to=to,
        orient=tk.HORIZONTAL,
        length=300,
        bg="#2c2c2c",
        fg=TEXT_COLOR,
        highlightthickness=0,
        troughcolor=SLIDER_COLOR,
        bd=0
    )
    slider.set(init)
    slider.pack(pady=5)
    return slider

scale_verde = make_slider(time_frame, "Verde:", 1, 20, 5)
scale_amarillo = make_slider(time_frame, "Amarillo:", 1, 10, 2)
scale_rojo = make_slider(time_frame, "Rojo:", 1, 20, 8)

# Botón salir
tk.Button(
    window,
    text="❌ Cerrar Aplicación",
    command=window.destroy,
    bg="#555",
    fg="white",
    font=BUTTON_FONT,
    relief="flat",
    width=25
).place(x=100, y=600, height=45)

# --- Iniciar bucle de polling ---
fetch_status()
window.mainloop()
