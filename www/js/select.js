$(document).ready(function() {
	
	$.ajax({
		type: "GET",
		url: "/fileListing.py",
		success: function(msg){
			for (let i = 0; i < msg.length; i++) {
				
				console.log("HERE");
				let ln_br = msg.indexOf('\n', i);
				let fileName = msg.slice(i, ln_br);
				$('#del_files').append('<option value="' + fileName + '">' + fileName + '</option>');
				i = ln_br;
			}
		}
	});
});