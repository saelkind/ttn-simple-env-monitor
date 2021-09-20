****** Changed default TTN v3 console Javascript payload decoder to:
****** Cut and paste stuff below into web interface
		  
function decodeUplink(input) {
  return {
    data: {
      message: String.fromCharCode.apply(null, input.bytes)
    },
    warnings: [],
    errors: []
  };
}