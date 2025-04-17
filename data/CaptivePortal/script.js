// =============================================
// 1. CONSTANTES E ELEMENTOS DA INTERFACE
// =============================================
const modal = {
    element: document.getElementById('modal'),
    title: document.getElementById('modal-title'),
    message: document.getElementById('modal-message'),
    closeBtn: document.getElementById('modal-close'),
    saveBtn: document.getElementById('save-wifi-credentials-btn')
};

const wifiForm = {
    ssidSelect: document.getElementById('ssid'),
    passwordField: document.getElementById('password-field'),
    passwordInput: document.getElementById('password'),
    connectBtn: document.getElementById('connect-btn'),
    dhcpCheckbox: document.getElementById('dhcp-checkbox'),
    saveBtn: document.getElementById('save-wifi-credentials-btn'),
    dhcpWrapper: document.getElementById('dhcp-wrapper'),
    refreshBtn: document.getElementById('refresh-wifi')
};

const staticIpForm = {
    mDnsInput: document.getElementById('mDns'),
    ipFields: document.querySelectorAll('.ip-group input[type="number"]'),
    saveConfigBtn: document.getElementById('save-wifi-config-btn')
};

// =============================================
// 2. FUNÇÕES UTILITÁRIAS
// =============================================
// Controle do Modal
function showModal(title, message) {
    modal.title.textContent = title;
    modal.message.textContent = message;
    modal.element.classList.add('active');
}

function closeModal() {
    modal.element.classList.remove('active');
}

// Validação de IP
function isValidOctet(value) {
    return value !== '' && !isNaN(value) && value >= 0 && value <= 255;
}

function isNotFirstOctetZero(octet) {
    return octet != 0;
}

function isNotLastOctetZeroOr255(octet) {
    return octet != 0 && octet != 255;
}

function isValidIP(ip1, ip2, ip3, ip4) {
    if (![ip1, ip2, ip3, ip4].every(isValidOctet)) return false;
    if (!isNotFirstOctetZero(ip1)) return false;
    if (!isNotLastOctetZeroOr255(ip4)) return false;
    if (ip1 == 127 && ip2 == 0 && ip3 == 0 && ip4 == 1) return false;
    if (ip1 == 255 && ip2 == 255 && ip3 == 255 && ip4 == 255) return false;
    return true;
}

function isValidSubnet(sn1, sn2, sn3, sn4) {
    if (![sn1, sn2, sn3, sn4].every(isValidOctet)) return false;
    
    const mask = (sn1 << 24) | (sn2 << 16) | (sn3 << 8) | sn4;
    if (mask === 0 || mask === 0xFFFFFFFF) return false;
    
    let foundZero = false;
    for (let i = 31; i >= 0; i--) {
        const bit = (mask >> i) & 1;
        if (bit === 0) foundZero = true;
        if (foundZero && bit === 1) return false;
    }
    return true;
}

function areInSameNetwork(ip, gateway, subnet) {
    const ipParts = ip.split('.').map(Number);
    const gwParts = gateway.split('.').map(Number);
    const snParts = subnet.split('.').map(Number);
    
    return ipParts.every((part, i) => (part & snParts[i]) === (gwParts[i] & snParts[i]));
}

// =============================================
// 3. FUNÇÕES PRINCIPAIS
// =============================================
// Função para buscar redes WiFi
async function scanWifi() {
    wifiForm.ssidSelect.innerHTML = '<option value="">Selecione uma rede...</option>';

    try {
        const response = await fetch('/scan-wifi');
        const networks = await response.json();

        networks.forEach(network => {
            const option = document.createElement('option');
            option.value = network.ssid;
            const lockIcon = network.open ? '🔓' : '🔒';
            const authStatus = network.open ? ' (Aberta)' : ' (Protegida)';
            option.textContent = `${lockIcon} ${network.ssid} ${authStatus} (${network.rssi}dBm)`;
            option.dataset.open = network.open;
            if (network.open) option.style.color = '#2ecc71';
            wifiForm.ssidSelect.appendChild(option);
        });
    } catch (error) {
        console.error('Erro ao buscar redes:', error);
        const option = document.createElement('option');
        option.textContent = '❌ Erro ao carregar redes';
        wifiForm.ssidSelect.appendChild(option);
    }
}

// Função para conectar ao WiFi
async function connectToWifi(e) {
    e.preventDefault();
    
    const ssid = wifiForm.ssidSelect.value;
    const password = wifiForm.passwordInput.value;
    const dhcp = wifiForm.dhcpCheckbox.checked;
    const isOpenNetwork = wifiForm.ssidSelect.options[wifiForm.ssidSelect.selectedIndex]?.dataset.open === 'true';

    if (!ssid) {
        showModal('Erro', 'Selecione uma rede Wi-Fi');
        return;
    }

    if (!isOpenNetwork && !password) {
        showModal('Erro', 'Digite a senha da rede Wi-Fi');
        return;
    }

    showModal('Conectando', `Conectando à rede ${ssid}...`);

    try {
        const response = await fetch('/connect-wifi', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ssid, password, dhcp })
        });

        if (!response.ok) throw new Error('Erro no servidor');
        
        const data = await response.json();
        showModal('Sucesso', data.message || 'Conectado com sucesso!');
        modal.saveBtn.classList.remove('hidden');
    } catch (error) {
        showModal('Erro', 'Falha na conexão: ' + error.message);
    }
}

// Função para salvar credenciais WiFi
async function saveWifiCredentials(e) {
    e.preventDefault();
    
    const ssid = wifiForm.ssidSelect.value;
    const password = wifiForm.passwordInput.value;
    const dhcp = wifiForm.dhcpCheckbox.checked;
    const isOpenNetwork = wifiForm.ssidSelect.options[wifiForm.ssidSelect.selectedIndex]?.dataset.open === 'true';

    if (!ssid) {
        showModal('Erro', 'Selecione uma rede Wi-Fi');
        return;
    }

    if (!isOpenNetwork && !password) {
        showModal('Erro', 'Digite a senha da rede Wi-Fi');
        return;
    }

    showModal('Salvando', 'Salvando credenciais Wi-Fi...');

    try {
        const response = await fetch('/save-wifi-credentials', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                ssid,
                password: isOpenNetwork ? '' : password,
                dhcp
            })
        });

        if (!response.ok) throw new Error('Erro no servidor');
        
        const data = await response.json();
        showModal('Sucesso', data.message || 'Credenciais salvas com sucesso!');
        
        setTimeout(() => {
            closeModal();
            // Resetar formulário se necessário
            wifiForm.ssidSelect.value = '';
            wifiForm.passwordInput.value = '';
            wifiForm.dhcpCheckbox.checked = true;
            wifiForm.passwordField.style.display = 'none';
        }, 2000);
    } catch (error) {
        showModal('Erro', 'Falha ao salvar: ' + error.message);
    }
}

// Função para salvar configuração de IP estático
async function saveStaticIpConfig() {
    const mDns = staticIpForm.mDnsInput.value;
    const ip = Array.from(staticIpForm.ipFields).map(field => field.value);
    modal.saveBtn.classList.add('hidden');
    
    if (!mDns || !/^[a-zA-Z0-9]+$/.test(mDns)) {
        showModal('Erro', 'Nome de rede inválido! Deve conter apenas letras e números.');
        return;
    }

    if (!isValidIP(...ip)) {
        showModal('Erro', 'Endereço IP inválido!\n- Cada octeto deve estar entre 1-254\n- Primeiro octeto não pode ser 0\n- Último octeto não pode ser 0 ou 255\n- Não pode ser 127.0.0.1');
        return;
    }

    const ipStr = ip.join('.');
    const gwStr = Array.from(document.querySelectorAll('.ip-group input[id^="gw"]')).map(f => f.value).join('.');
    const snStr = Array.from(document.querySelectorAll('.ip-group input[id^="sn"]')).map(f => f.value).join('.');

    if (!areInSameNetwork(ipStr, gwStr, snStr)) {
        showModal('Erro', 'IP e Gateway não estão na mesma rede de acordo com a máscara fornecida!');
        return;
    }

    showModal('Salvando', 'Aguarde enquanto salvamos as configurações...');

    try {
        const response = await fetch('/save-wifi-config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                mDns,
                ip: ipStr,
                gateway: gwStr,
                subnet: snStr
            })
        });

        if (!response.ok) throw new Error('Erro no servidor');
        
        const data = await response.json();
        showModal('Sucesso', data.message || 'Configuração salva com sucesso!');
    } catch (error) {
        showModal('Erro', 'Falha ao salvar: ' + error.message);
    }
}

// =============================================
// 4. CONFIGURAÇÃO DE EVENT LISTENERS
// =============================================
// Menu responsivo
document.querySelector('.menu-toggle').addEventListener('click', () => {
    document.querySelector('nav ul').classList.toggle('active');
});

// Controle do formulário WiFi
wifiForm.dhcpCheckbox.addEventListener('change', function() {
    staticIpForm.ipFields.forEach(field => {
        field.disabled = this.checked;
    });
});

wifiForm.ssidSelect.addEventListener('change', function() {
    const selectedOption = this.options[this.selectedIndex];
    const hasValue = this.value !== '';
    const isOpen = selectedOption?.dataset.open === 'true';

    // Esconde tudo se não tiver valor selecionado
    if (!hasValue) {
        wifiForm.passwordField.style.display = 'none';
        wifiForm.connectBtn.style.display = 'none';
        wifiForm.dhcpWrapper.style.display = 'none';
        return;
    }

    // Mostra elementos (com tratamento especial para o campo de senha)
    wifiForm.connectBtn.style.display = 'block';
    wifiForm.dhcpWrapper.style.display = 'block';
    wifiForm.passwordField.style.display = isOpen ? 'none' : 'block';
});

// Configuração de IP estático
staticIpForm.mDnsInput.addEventListener('input', function() {
    this.value = this.value.replace(/[^a-zA-Z0-9]/g, '');
});

staticIpForm.mDnsInput.addEventListener('keypress', function(e) {
    if (!/[a-zA-Z0-9]/.test(e.key)) e.preventDefault();
});

staticIpForm.ipFields.forEach((input, index) => {
    input.addEventListener('change', function() {
        if (this.value > 255) this.value = 255;
        if (this.value < 0) this.value = 0;
    });

    input.addEventListener('keydown', function(e) {
        if (['.', 'Enter', ',', 'Period'].includes(e.key)) {
            e.preventDefault();
            const nextIndex = index + 1;
            if (nextIndex < staticIpForm.ipFields.length) {
                staticIpForm.ipFields[nextIndex].focus();
                staticIpForm.ipFields[nextIndex].value = '';
            }
        }
    });
});

// Eventos principais
modal.closeBtn.addEventListener('click', closeModal);
document.getElementById('credentials').addEventListener('submit', connectToWifi);
wifiForm.saveBtn.addEventListener('click', saveWifiCredentials);
wifiForm.refreshBtn.addEventListener('click', scanWifi);
staticIpForm.saveConfigBtn.addEventListener('click', saveStaticIpConfig);

// Inicialização
window.addEventListener('DOMContentLoaded', () => {
    scanWifi();
});