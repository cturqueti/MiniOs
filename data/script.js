document.addEventListener('DOMContentLoaded', function() {
    // Busca redes WiFi disponíveis
    fetch('/scan')
        .then(response => response.json())
        .then(networks => {
            const select = document.getElementById('ssid');
            networks.forEach(network => {
                const option = document.createElement('option');
                option.value = network.ssid;
                option.textContent = network.ssid + (network.rssi ? ` (${Math.round(network.rssi)}%)` : '');
                select.appendChild(option);
            });
        })
        .catch(error => console.error('Error:', error));

    // Envia formulário
    const form = document.getElementById('wifi-form');
    form.addEventListener('submit', function(e) {
        e.preventDefault();
        
        const formData = new FormData(form);
        fetch('/connect', {
            method: 'POST',
            body: formData
        })
        .then(response => response.text())
        .then(data => {
            alert(data);
        })
        .catch(error => {
            console.error('Error:', error);
            alert('Erro ao conectar!');
        });
    });
});