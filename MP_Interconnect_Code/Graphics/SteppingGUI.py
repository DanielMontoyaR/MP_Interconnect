import tkinter as tk
import os
from tkinter import *
from tkinter import filedialog
import tkinter.font as tkFont
from PIL import Image, ImageTk, ImageSequence











pipe_path = "tmp/continue_pipe"
def send_signal():
    if not os.path.exists(pipe_path):
        print("Pipe no longer exists.")
        button.config(state=tk.DISABLED)
        return
    try:
        with open(pipe_path, "w") as f:
            f.write("continue\n")
    except OSError as e:
        print(f"Error writing to pipe: {e}")
        button.config(state=tk.DISABLED)

"""
root = tk.Tk()
button = tk.Button(root, text="Continue", command=send_signal)
button.pack()
root.mainloop()
"""



def main():

    def animar_gif(ind):
        frame = frames[ind]
        ind = (ind + 1) % len(frames)
        fondo_label.configure(image=frame)
        window.after(100, animar_gif, ind)  # ajusta el tiempo según el GIF

    # Creaccion de la ventana y sus caracteristicas
    window = Tk() 
    window.configure(background="Light Grey") 
    window.title("Stepping GUI") 
    window.geometry("1700x900") 
    window.resizable(FALSE,FALSE)


    gif = Image.open("siuuu-ronaldo.gif")

    frames = [ImageTk.PhotoImage(frame.copy().resize((1700,900)).convert("RGBA")) for frame in ImageSequence.Iterator(gif)]

    fondo_label = tk.Label(window)
    fondo_label.place(x=0, y=0, relwidth=1, relheight=1)

    animar_gif(0)

    textMemory = Label(window, text = "Memory", background = "White")
    textMemory.place(x = 10, y = 50)


    def select_memory():
        #print("Mem:", opcion)

        opcion=mem_select.get()

        if(opcion == "Shared Memory"):
            archive = "../MemoryAndCache/shared_memory.txt"
        else:
            archive = f"../MemoryAndCache/{opcion}_cache.txt"

        try:
            with open(archive, "r", encoding="utf-8") as archivo:
                contenido = archivo.read()
                text_box.config(state=tk.NORMAL)
                text_box.delete(1.0, tk.END)
                text_box.insert(tk.END, contenido)
                text_box.config(state=tk.DISABLED)
        except FileNotFoundError:
            text_box.config(state=tk.NORMAL)
            text_box.delete(1.0, tk.END)
            text_box.insert(tk.END, f"Archivo no encontrado: {archive}")
            text_box.config(state=tk.DISABLED)
        
        #Call to update function
        #window.after(1000, select_memory)



    mem = ["Shared Memory", "PE0","PE1","PE2","PE3","PE4","PE5","PE6","PE7"]
    
    mem_select = StringVar()
    mem_select.set(mem[0])
    mem_menu = OptionMenu(window, mem_select, *mem, command = lambda _: select_memory())
    mem_menu.place(x=10, y=100)


    text_box = tk.Text(window, wrap=tk.NONE)
    #text_box = tk.Text(window, wrap=tk.NONE, font=("Arial", 10))
    scroll_y = tk.Scrollbar(window, orient=tk.VERTICAL, command=text_box.yview)
    scroll_x = tk.Scrollbar(window, orient=tk.HORIZONTAL, command=text_box.xview)

    #text_box = tk.Text(window, wrap=tk.NONE, font=("Arial", 8))
    text_box.config(yscrollcommand=scroll_y.set, xscrollcommand=scroll_x.set, state=tk.DISABLED)
    
    text_box.place(x=0,y=200, width=750, height=600)
    # Scroll vertical al lado derecho del cuadro
    scroll_y.place(x=750, y=200, height=600)

    # Scroll horizontal debajo del cuadro
    scroll_x.place(x=0, y=800, width=750)


    
    #../PE_logs

    



    textLogs = Label(window, text = "Logs", background = "White")
    textLogs.place(x = 1500, y = 50)

    
    def select_log():
        opcion = log_select.get()

        archive = f"../PE_logs/{opcion}_logs.txt"

        try:
            with open(archive, "r", encoding="utf-8") as archivo:
                contenido = archivo.read()
                log_box.config(state=tk.NORMAL)
                log_box.delete(1.0, tk.END)
                log_box.insert(tk.END, contenido)
                log_box.config(state=tk.DISABLED)
        except FileNotFoundError:
            log_box.config(state=tk.NORMAL)
            log_box.delete(1.0, tk.END)
            log_box.insert(tk.END, f"Archivo no encontrado: {archive}")
            log_box.config(state=tk.DISABLED)

    logs = mem[1:]

    log_select = StringVar()
    log_select.set(logs[0])
    log_menu = OptionMenu(window, log_select, *logs, command =  lambda _: select_log())
    log_menu.place(x=1500, y=100)


    log_box = tk.Text(window, wrap=tk.NONE)
    #text_box = tk.Text(window, wrap=tk.NONE, font=("Arial", 10))
    log_scroll_y = tk.Scrollbar(window, orient=tk.VERTICAL, command=log_box.yview)
    log_scroll_x = tk.Scrollbar(window, orient=tk.HORIZONTAL, command=log_box.xview)

    #text_box = tk.Text(window, wrap=tk.NONE, font=("Arial", 8))
    log_box.config(yscrollcommand=log_scroll_y.set, xscrollcommand=log_scroll_x.set, state=tk.DISABLED)
    
    log_box.place(x=900,y=200, width=750, height=600)
    # Scroll vertical al lado derecho del cuadro
    log_scroll_y.place(x=1650, y=200, height=600)

    # Scroll horizontal debajo del cuadro
    log_scroll_x.place(x=900, y=800, width=750)


    def update_text():
        select_log()
        select_memory()

    def continue_ops():
        print("Continuing Operations")

    Update = Button(window, text='Update', fg='black', bg='white', command=update_text, height=1, width=7) 
    Update.place(x=800,y=50)

    Continue = Button(window, text='Continue', fg='black', bg='white', command=continue_ops, height=1, width=7) 
    Continue.place(x=800,y=100)

    select_log()
    select_memory()
    window.mainloop() 

main()