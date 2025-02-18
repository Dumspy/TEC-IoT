document.getElementById('wifiForm').addEventListener('submit', async function(event) {
    event.preventDefault();
    const form = event.target;
    const formData = new FormData(form);
    const params = new URLSearchParams(formData);

    try {
        const response = await fetch('/save', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/x-www-form-urlencoded'
            },
            body: params.toString()
        });

        const messageDiv = document.getElementById('message');
        if (response.status === 200) {
            messageDiv.textContent = 'Wi-Fi settings saved successfully!';
            messageDiv.style.color = 'green';
            messageDiv.style.border = '1px solid green';
        } else {
            messageDiv.textContent = 'Failed to save Wi-Fi settings.';
            messageDiv.style.color = 'red';
            messageDiv.style.border = '1px solid red';
        }
    } catch (error) {
        const messageDiv = document.getElementById('message');
        messageDiv.textContent = 'An error occurred while saving Wi-Fi settings.';
        messageDiv.style.color = 'red';
        messageDiv.style.border = '1px solid red';
    }
});
