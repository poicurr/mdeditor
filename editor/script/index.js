window.onload = (ev) => {

  let fileSelector = document.querySelector("#fileSelector");
  let fileSaveBtn  = document.querySelector("#savefile");
  let editor = document.querySelector(".lpanel");
  let result = document.querySelector(".target");

  editor.addEventListener("input", (ev) => {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/update", true);
    xhr.onload = (ev) => {
      if (xhr.readyState == 4 && xhr.status == 200) {
        result.innerHTML = xhr.responseText;
      }
    }
    xhr.onerror = (ev) => {
      console.error(xhr.statusText);
    }
    xhr.send(ev.target.value);
  });

  fileSelector.addEventListener("change", (ev) => {
    let reader = new FileReader();
    let file = ev.target.files[0];
    reader.readAsText(file);
    reader.onload = (ev) => {
      editor.value = reader.result;
      editor.dispatchEvent(new Event('input'));
    }
  });

  fileSaveBtn.addEventListener("click", (ev) => {
    const a = document.createElement("a");
    a.href = "data:text/plain," + encodeURIComponent(editor.value);
    a.download = "unnamed.md";
    a.style.display = "none";
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
  });

}
