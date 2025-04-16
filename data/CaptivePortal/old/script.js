document.addEventListener('DOMContentLoaded', function() {
    const ipMode = document.getElementById('ip-mode');
    const ipStaticFields = document.getElementById('ip-static-fields');
    const connectBtn = document.getElementById('connect-btn');
    const status = document.getElementById('status');
    
    // Carrega redes WiFi disponíveis
    fetch('/scan')
        .then(response => response.json())
        .then(networks => {
            const ssidSelect = document.getElementById('ssid');
            networks.forEach(network => {
                const option = document.createElement('option');
                option.value = network.ssid;
                option.textContent = network.ssid + ` (${network.rssi} dBm)`;
                ssidSelect.appendChild(option);
            });
        });
    
    // Mostra/oculta campos de IP estático conforme seleção
    ipMode.addEventListener('change', function() {
        ipStaticFields.style.display = this.value === 'static' ? 'block' : 'none';
    });
    
    // Conectar à rede WiFi
    connectBtn.addEventListener('click', function() {
        const ssid = document.getElementById('ssid').value;
        const password = document.getElementById('password').value;
        const hostname = document.getElementById('hostname').value;
        const ipModeValue = ipMode.value;
        
        if (!ssid) {
            status.textContent = "Por favor, selecione uma rede WiFi";
            status.style.color = "red";
            return;
        }
        
        if (!hostname) {
            status.textContent = "Por favor, informe um nome para o dispositivo";
            status.style.color = "red";
            return;
        }
        
        // Preparar dados para envio
        const postData = {
            ssid: ssid,
            password: password,
            hostname: hostname,
            ipMode: ipModeValue
        };
        
        // Adicionar dados de IP estático se necessário
        if (ipModeValue === 'static') {
            postData.staticIp = document.getElementById('static-ip').value;
            postData.gateway = document.getElementById('static-gateway').value;
            postData.subnet = document.getElementById('static-subnet').value;
            
            // Validar IPs
            const ipRegex = /^([0-9]{1,3}\.){3}[0-9]{1,3}$/;
            if (!ipRegex.test(postData.staticIp) || 
                !ipRegex.test(postData.gateway) || 
                !ipRegex.test(postData.subnet)) {
                status.textContent = "Por favor, insira endereços IP válidos";
                status.style.color = "red";
                return;
            }
        }
        
        // Enviar configuração para o ESP32
        connectBtn.disabled = true;
        status.textContent = "Conectando...";
        status.style.color = "blue";
        
        fetch('/connect', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(postData)
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                status.textContent = "Conectado com sucesso! IP: " + (data.ip || "DHCP");
                status.style.color = "green";
            } else {
                throw new Error(data.message || "Erro na conexão");
            }
        })
        .catch(error => {
            status.textContent = "Erro: " + error.message;
            status.style.color = "red";
        })
        .finally(() => {
            connectBtn.disabled = false;
        });
    });
});