# mybash
## Equipo
- Lilen Bena
- Camila Micaela Castillo Cortés
- Nicolás Mansutti
- Pietro Palombini

## Funcionamiento
Funciona exactamente como esperarías que funcione un shell (simple). Algunos detalles pueden ser:
- Se puede salir con Ctrl+D o con exit en el prompt
- Se puede interrumpir un comando que se está ejecutando y salir de mybash con Ctrl+C
- Se pueden usar una cantidad arbitraria de pipes
- El manejo de los errores de sintaxis es robusto, muesta un error cuando se escribe un comando inválido.
    - Considerar comandos del tipo `sleep 10 & echo hi` como inválidos es intencional. Es más idiomático añadir un salto de línea después de un `&`.

## Checks
- Los targets del makefile para correr los tests siguen funcionando como en el kickstart, pero hay targets adicionales.
- Correr `make sanitize-all` ejecuta todos los tests y el leaktest con AddressSanitizer, LeakSanitizer y UndefinedBehaviorSanitizer. Esta es la prueba más comprensiva.
    - Hay un memory leak proveniente de lexer.o que no pudimos corregir al no tener el código, por lo que lo suprimimos.
    - Para implementar esto hubo que arreglar un memory leak en uno de los tests, por lo que los tests no están intactos. De todas formas, no sacamos ni alteramos el funcionamiento de ningún test más allá de arreglar el memory leak.
