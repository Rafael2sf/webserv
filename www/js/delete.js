
let buttonId = document.getElementById("btn");

function handleDelete() {

	let select = document.getElementById("del_files");
	try {
		let fileSelected = select.options[select.selectedIndex].value;
		$.ajax({
			type: "DELETE",
			url: "/delete.py",
			data: {file: fileSelected},
			success: function(){
				alert("File " + fileSelected + " erased!");
				location.reload();
			},
			error: function( jqXHR, textStatus, errorThrown) {
				alert("ERROR: " + errorThrown);
			}
		});
	} 
	catch (error) {
		alert("File does not exist!");
	}
}

buttonId.addEventListener("click", handleDelete);