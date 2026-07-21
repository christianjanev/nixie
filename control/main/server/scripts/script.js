async function activateLED()
{
    const url = window.location.href;
    try {
        const response = await fetch(url + "activate", {
	    method: "POST"
	});
    }
    catch (error)
    {
	console.error(error.message);
    }
    console.log(window.location.href);
}
