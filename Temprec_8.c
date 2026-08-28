En Linux no existen unas "credenciales para acceder al núcleo (kernel)" como tal. El acceso al kernel se gestiona a través de privilegios de usuario, y la forma estándar de obtenerlos es con el comando sudo.

Para ejecutar comandos que interactúan con el kernel (ej. modprobe, dmesg, configurar parámetros con sysctl), se usan estos métodos:

· sudo comando: Ejecuta un comando con privilegios de superusuario (root). Te pedirá tu propia contraseña, no una del kernel. Para que funcione, tu usuario debe estar autorizado en el archivo /etc/sudoers.
· Usuario root: Es la cuenta del superusuario con control total sobre el sistema. Para usarla, necesitas conocer la contraseña de root. No se recomienda usarla para tareas cotidianas.
· sudo -i o sudo su: Inician una sesión de shell como root. Con sudo -i usas tu contraseña; con sudo su puedes necesitar la de root.

En resumen: las "credenciales" no son para el kernel, sino para tu cuenta de usuario (con sudo) o para la cuenta root. El kernel en sí no tiene un sistema de login independiente.
