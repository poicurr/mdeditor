window.onload = (ev) => {

  let reader = new FileReader();

  let fileSelector = document.querySelector("#fileSelector");
  let editor = document.querySelector(".lpanel");
  let result = document.querySelector(".target");

  editor.addEventListener("input", (ev) => {
    $.ajax({
      url: "/save",
      type: "post",
      dataType: "text",
      data: { "name": encodeURI(ev.target.value) },
    }).done((data) => {
      result.innerHTML = decodeURI(data);
    }).fail(function(XMLHttpRequest, textStatus, errorThrown) {
      console.log(textStatus);
    })
  });

  fileSelector.addEventListener("change", (ev) => {
    let file = ev.target.files[0];
    reader.readAsText(file);
    reader.onload = (ev) => {
      editor.value = reader.result;
      editor.dispatchEvent(new Event('input'));
    }
  });

}
