# Retransmitter
An app that works in user space: 
* handles messages from MQTT broker, retransmits them to STM32 via nRF24
* handles messages from STM32 via nRF24 and retransmits them to MQTT broker