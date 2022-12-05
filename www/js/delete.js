
let buttonId = document.getElementById("btn");

function handleDelete() {

	let select = document.getElementById("del_files");
	try {
		let fileSelected = select.options[select.selectedIndex].value;
		$.ajax({
			type: "DELETE",
			url: "/scripts/delete.py",
			data: {file: fileSelected},
			success: function(msg){
				$(document).html(msg);
				alert("File " + fileSelected + " erased!");
				location.reload();
			}
		});
	} 
	catch (error) {
		alert("File does not exist!");
	}
}

buttonId.addEventListener("click", handleDelete);