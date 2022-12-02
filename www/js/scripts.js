$(document).ready(function() {
    
	$.ajax({
		type: "DELETE",
		url: "/scripts/delete.py",
		data: {file: "just/posts/python.webp"},
		success: function(msg){
			$("#response").html(msg);
			// alert("Data Deleted!");
		}
	});
});
