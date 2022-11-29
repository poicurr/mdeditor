window.onload = (ev) => {

  let reader = new FileReader();

  let fileSelector = document.querySelector("#fileSelector");
  let editor = document.querySelector(".lpanel");
  let result = document.querySelector(".target");

  editor.addEventListener("input", (ev) => {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/save", true);
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
    let file = ev.target.files[0];
    reader.readAsText(file);
    reader.onload = (ev) => {
      editor.value = reader.result;
      editor.dispatchEvent(new Event('input'));
    }
  });

}
