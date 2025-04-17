
// Controle do modal
const modalController = (function() {
    const modal = document.getElementById('modal');
    const modalTitle = document.getElementById('modal-title');
    const modalMessage = document.getElementById('modal-message');
    
    return {
        open: (title, message) => {
            modalTitle.textContent = title;
            modalMessage.textContent = message;
            modal.classList.add('active'); // Use 'active' em vez de 'hidden'
        },
        close: () => {
            modal.classList.remove('active');
        }
    };
})();

function showModal(title, message) {
    modalController.open(title, message);
}

// Menu responsivo
document.querySelector('.menu-toggle').addEventListener('click', function() {
    document.querySelector('nav ul').classList.toggle('active');
});

// Controle do formulário WiFi
const ssidSelect = document.getElementById('ssid');
const passwordField = document.getElementById('password-field');
const connectBtn = document.getElementById('connect-btn');
const dhcpCheckbox = document.getElementById('dhcp-checkbox');
const saveWiFiCredentials = document.getElementById('save-wifi-credentials-btn');
const dhcpWrapper = document.getElementById('dhcp-wrapper');

// Ativa/desativa campos IP estático
dhcpCheckbox.addEventListener('change', function() {
    const ipFields = document.querySelectorAll('#static-ip-form input[type="number"]');
    ipFields.forEach(field => {
        field.disabled = this.checked;
    });
});

// Mostra senha quando rede é selecionada
ssidSelect.addEventListener('change', function() {
    if (this.value) {
        passwordField.classList.remove('hidden');
        connectBtn.classList.remove('hidden');
        dhcpWrapper.classList.remove('hidden');
    } else {
        passwordField.classList.add('hidden');
        connectBtn.classList.add('hidden');
        dhcpWrapper.classList.add('hidden');
    }
});

// Busca redes WiFi disponíveis
window.addEventListener('DOMContentLoaded', function() {
    const ssidSelect = document.getElementById('ssid'); // Certifique-se que este ID existe no seu HTML
    const refreshButton = document.getElementById('refresh-wifi');

    function scanWifi() {
        // Limpa as opções antigas
        ssidSelect.innerHTML = '<option value="">Selecione uma rede...</option>';

        fetch('/scan-wifi')
            .then(response => response.json())
            .then(networks => {
                networks.forEach(network => {
                    const option = document.createElement('option');
                    option.value = network.ssid;
                    
                    // Ícone e texto baseado no status da rede
                    const lockIcon = network.open ? '🔓' : '🔒';
                    const authStatus = network.open ? ' (Aberta)' : ' (Protegida)';
                    
                    option.textContent = `${lockIcon} ${network.ssid} ${authStatus} (${network.rssi}dBm)`;
                    
                    // Adiciona atributo data-open para uso posterior
                    option.dataset.open = network.open;
                    
                    // Opcional: estilo diferente para redes abertas
                    if (network.open) {
                        option.style.color = '#2ecc71'; // Verde para redes abertas
                    }
                    
                    ssidSelect.appendChild(option);
                });
            })
            .catch(error => {
                console.error('Erro ao buscar redes:', error);
                const option = document.createElement('option');
                option.textContent = '❌ Erro ao carregar redes';
                ssidSelect.appendChild(option);
            });
    }

    // Executa ao carregar a página
    scanWifi();

    refreshButton.addEventListener('click', scanWifi);
});


// Envio do formulário WiFi
document.getElementById('wifi-form').addEventListener('submit', function(e) {
    e.preventDefault();
    saveWiFiCredentials.classList.add('hidden');

    modalController.open('Aguarde', `Conectando à rede ${ssidSelect.value}...`);
    
    const formData = {
        ssid: ssidSelect.value,
        password: document.getElementById('password').value,
        dhcp: dhcpCheckbox.checked
    };
    
    fetch('/connect-wifi', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
    .then(response => response.json())
    .then(data => {
        showModal('Sucesso', data.message || 'Conectado com sucesso!');
        saveWiFiCredentials.classList.remove('hidden');
    })
    .catch(error => {
        showModal('Erro', 'Falha na conexão: ' + error.message);
    });
});

// Envio do comando para salvar as credenciais WiFi
document.getElementById('save-wifi-credentials-btn').addEventListener('click', async function() {
    
    showModal('Salvando...', 'Aguarde enquanto salvamos as configurações');

    try {
        const formData = {
            ssid: ssidSelect.value,
            password: document.getElementById('password').value,
            dhcp: dhcpCheckbox.checked
        };
        
        const response = await fetch('/save-wifi-credentials', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(formData)
        });
        
        const data = await response.json();
        showModal('Sucesso', data.message || 'Configurações salvas!');
        
        await new Promise(resolve => setTimeout(resolve, 2000));
        window.location.reload();
        
    } catch (error) {
        showModal('Erro', 'Falha ao salvar: ' + error.message);
    }

});

// Envio do formulário IP estático
document.getElementById('save-wifi-config-btn').addEventListener('click', function() {
    showModal('Salvando...', 'Aguarde enquanto salvamos as configurações');
    
    const formData = {
        mDns: document.getElementById('mDns').value,
        ip: `${document.getElementById('ip1').value}.${document.getElementById('ip2').value}.${document.getElementById('ip3').value}.${document.getElementById('ip4').value}`,
        gateway: `${document.getElementById('gw1').value}.${document.getElementById('gw2').value}.${document.getElementById('gw3').value}.${document.getElementById('gw4').value}`,
        subnet: `${document.getElementById('sn1').value}.${document.getElementById('sn2').value}.${document.getElementById('sn3').value}.${document.getElementById('sn4').value}`
    };
    
    fetch('/save-wifi-config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(formData)
    })
    .then(response => response.json())
    .then(data => {
        showModal('Sucesso', data.message || 'Configuração salva!');
    })
    .catch(error => {
        showModal('Erro', 'Falha ao salvar: ' + error.message);
    });
});

// Funções auxiliares// Fechar modal ao clicar no botão ou fora
document.getElementById('modal-close').addEventListener('click', modalController.close);

// document.getElementById('modal').addEventListener('click', function(e) {
//     if (e.target === this) modalController.close();
// });

ssidSelect.addEventListener('change', function() {
    const selectedOption = this.options[this.selectedIndex];
    const isOpen = selectedOption.dataset.open === 'true';
    
    if (isOpen) {
        // Esconde campo de senha para redes abertas
        document.getElementById('password-field').style.display = 'none';
    } else {
        // Mostra campo de senha para redes protegidas
        document.getElementById('password-field').style.display = 'block';
    }
});