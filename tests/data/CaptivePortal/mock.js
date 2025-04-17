// retirar antes da versão final
// Interceptar chamadas fetch
const originalFetch = window.fetch;
window.fetch = async (url, options) => {
    console.log(`[mock] fetch para: ${url}`);
    
    if (url === '/scan-wifi') {
        return new Response(JSON.stringify([
            { 
                ssid: 'sideout', 
                rssi: -42,
                authMode: 'WPA2', // ou isOpen: false
                open: false
            },
            { 
                ssid: 'guest_wifi', 
                rssi: -70,
                authMode: 'WPA', // ou isOpen: false
                open: false
            },
            { 
                ssid: 'guest', 
                rssi: -70,
                authMode: 'OPEN', // ou isOpen: true
                open: true
            }
        ]), {
            status: 200,
            headers: { 'Content-Type': 'application/json' }
        });
    }

    if (url === '/connect-wifi') {
        console.log(`[mock] Conectando à rede: ${JSON.parse(options.body).ssid}`);
        
        await new Promise(resolve => setTimeout(resolve, 2000));
        
        return new Response(JSON.stringify({
            message: 'Conectado com sucesso à rede ' + JSON.parse(options.body).ssid
        }), {
            status: 200,
            headers: { 'Content-Type': 'application/json' }
        });
    }

    if (url === '/save-wifi-config') {
        console.log(`[mock] Salvando configurações de WiFi: ${JSON.parse(options.body).ip}`);
        return new Response(JSON.stringify({

            message: 'Configurações salvas com sucesso!'
        }), {
            status: 200,
            headers: { 'Content-Type': 'application/json' }
        });
    }
    if (url === '/save-wifi-credentials') {
        console.log(`[mock] Salvando configurações de WiFi: ${JSON.parse(options.body).ssid}`);
        return new Response(JSON.stringify({
            message: 'Configurações salvas com sucesso!'
        }), {
            status: 200,
            headers: { 'Content-Type': 'application/json' }
        });
    }

    // Fallback: chamada real
    return originalFetch(url, options);
};
// retirar antes da versão final
