$(function () {
    // we use an inline data source in the example, usually data would
    // be fetched from a server
    var data = [], totalPoints = 300;
	var timer;
	var time_started;
	var button_start=document.getElementById("starttimer");
	var button_stop=document.getElementById("stoptimer");
	var start_time,stop_time;

	$("#start_time").val("2013-09-14T00:00");
	$("#stop_time").val("23:59");
	button_start.onclick=function()
	{
		time_started=new Date();
		updateData();
		this.disabled=true;
	}

	button_stop.onclick=function()
	{
		clearTimeout(timer);
		button_start.disabled=false;
	}

	function onDataReceived(result,status,xhr)
	{
		var series=JSON.parse(result);
		series[0]=series[0]*1000;
		data.push(series);
	}

	function getData()
	{
		dataurl="/cgi-bin/tempsensor.cgi?action=queryTemperature"
		if(data.length==totalPoints)
		{
			data=data.slice(1);//slice an array from index of 1
		}
		var i=(data.length<2)?2:1;
		for(var j=0;j<i;j++)
		{
			$.ajax({
				async:false,
				timeout:3000,
				url: dataurl,
				method: 'GET',
				//dataType: 'json',
				success: onDataReceived
			});
		}
		return data;//will be exected before $.ajax() if async is true
	}

    // setup control widget
    var updateInterval = 3;
    $("#updateInterval").val(updateInterval).change(function () {
        var v = $(this).val();
		if (v && !isNaN(+v)) 
		{//isNaN() determines whether a value is an illegal number(Not-a-Number)
            updateInterval = +v;
            if (updateInterval < 1)
                updateInterval = 1;
            if (updateInterval > 2000)
                updateInterval = 2000;
            $(this).val("" + updateInterval);
        }
    });

    // setup plot
    var options = 
	{
		series: 
		{ 
			shadowSize: 0,
			lines:{show:true},
			points:{show:true}
		}, // drawing is faster without shadows
		yaxis: {min: 20, max: 40 },
		xaxis: 
		{
			//min:,max:,
			show: false ,mode: "time",
			minTickSize:[1,"second"],
			ticks:60
			/*timeformat:"%M:%S"*/
		},
		grid: { hoverable: true, clickable: true }
	};
	var plot = $.plot($("#placeholder"), 
			[{label:"T(t)",data: getData()} ],
			options);

    function updateData() 
	{
		plot.setData([ getData() ]);
		// since the axes don't change, we don't need to call plot.setupGrid()
		plot.setupGrid();
		plot.draw();
		//plot=$.plot($("#placeholder"),
		//		[{label:"T(t)",data:getData()}],options);

		timer=setTimeout(update, 1000*updateInterval); //calling itself
		var date=time_started.getFullYear()+"-";
		if(time_started.getMonth()<10)
		{
			date+=""+0+(time_started.getMonth()+1)+"-";
		}
		else
		{
			date+=(time_started.getMont()+1);
		}
		if(time_started.getDate()<0)
		{
			date+=""+0+time_started.getDate();
		}
		else
		{
			date+=time_started.getDate();
		}
		var t1=time_started.getHours()+":"+time_started.getMinutes();
		var d=new Date();
		var t2=d.getHours()+":"+(d.getMinutes()+1);
		$.ajax({
			async:true,
			timeout:3000,
			url:"/cgi-bin/tempsensor.cgi?action=get_extrema_temp&date="+date+"&t1="+t1+"&t2="+t2,
			method: 'GET',
			dataType: 'json',
			success:function(result,status,xhr){
				$("#max_temp").val("Max: "+result[0]);
				$("#min_temp").val("Min: "+result[1]);
			}
		});

    }

	$("#extrema_temp").click(function()
	{
		start_time=$("#start_time").val();
		stop_time=$("#stop_time").val();
		if(start_time==""||stop_time=="")
		{
			return;
		}
		else
		{
			var date=start_time.slice(0,10);
			var t1=start_time.slice(11,16);
			var t2=stop_time;

			$.ajax({
					async:true,
					timeout:3000,
					url:"/cgi-bin/tempsensor.cgi?action=get_extrema_temp&date="+date+"&t1="+t1+"&t2="+t2,
					method: 'GET',
					dataType: 'json',
					success:function(result,status,xhr){
						$("#max_temp").val("Max: "+result[0]);
						$("#min_temp").val("Min: "+result[1]);
					}
				});
		}

		return ;
	});

	//updateData();
});

