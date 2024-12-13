import { useEffect, useState } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer } from 'recharts';

function Graph({ graphInfo }) {
	const [chartData, setChartData] = useState([]);
	const [time, setTime] = useState(0); // Time counter to use for the x-axis
	const [isFrozen, setIsFrozen] = useState(true); // State for freezing/unfreezing the graph

 	// Handle graph updates only if the graph is not frozen
	useEffect(() => {
		if (graphInfo && !isFrozen) {
		const currentTime = time;  // Use current time counter
		setTime(prevTime => prevTime + 1);  // Increment the time by 1 for the next data point

		// Update chart data with new graph info and the time value
		setChartData(prevData => {
			const newData = [
			...prevData,
			{
				time: currentTime,  // Set time as the x-axis value
				xPWM: graphInfo.xPWM,
				yPWM: graphInfo.yPWM,
				zPWM: graphInfo.zPWM,
				yawPWM: graphInfo.yawPWM,
				xPID1: graphInfo.xPID1,
				yPID1: graphInfo.yPID1,
				zPID1: graphInfo.zPID1,
				yawPID1: graphInfo.yawPID1,
				xPID2: graphInfo.xPID2,
				yPID2: graphInfo.yPID2,
				zPID2: graphInfo.zPID2,
				yawPID2: graphInfo.yawPID2,
				xPosSetpoint: graphInfo.xPosSetpoint,
				yPosSetpoint: graphInfo.yPosSetpoint,
				zPosSetpoint: graphInfo.zPosSetpoint,
				xPos: graphInfo.xPos,  // Add position data
				yPos: graphInfo.yPos,
				zPos: graphInfo.zPos,
				yawPos: graphInfo.yawPos
			}
			];

			// Keep only the last 10 seconds of data
			// if (newData.length > 10) {
			//   newData.shift(); // Remove the oldest entry
			// }
			return newData;
		});
		}
	}, [graphInfo, time, isFrozen]);  // Add isFrozen to the dependency array

	// Toggle the freeze state
	const handleFreezeToggle = () => {
		setIsFrozen(prevState => !prevState);
	};

	const saveToFile = () => {
		const dataStr = JSON.stringify(chartData, null, 2); // Convert chartData to a formatted JSON string
		const blob = new Blob([dataStr], { type: 'application/json' }); // Create a Blob with JSON data
		const url = URL.createObjectURL(blob); // Create a temporary URL for the Blob
		const link = document.createElement('a'); // Create a link element
		link.href = url;
		link.download = 'chartData.json'; // Set the filename
		document.body.appendChild(link);
		link.click(); // Trigger the download
		document.body.removeChild(link); // Clean up the link element
	};

	return (
	<div>
		{/* Freeze toggle switch and save graph */}
		<label>
			<input type="checkbox" checked={isFrozen} onChange={handleFreezeToggle} />
			Freeze Graph
		</label>
		<button onClick={saveToFile}> - Save Data to File</button>

		{/* Setpoints Graph */}
		<ResponsiveContainer width="100%" height={200} style={{ backgroundColor: 'black' }}>
			<LineChart data={chartData}>
			<CartesianGrid strokeDasharray="3 3" vertical={false} />
			<XAxis dataKey="time" />
			<YAxis domain={[-1, 1]} />
			<Tooltip />
			<Legend />
			<Line type="monotone" dataKey="xPosSetpoint" stroke="red" />
			<Line type="monotone" dataKey="yPosSetpoint" stroke="blue" />
			<Line type="monotone" dataKey="zPosSetpoint" stroke="green" />
			</LineChart>
		</ResponsiveContainer>

		{/* Position Values Graph */}
		<ResponsiveContainer width="100%" height={200} style={{ backgroundColor: 'black' }}>
			<LineChart data={chartData}>
			<CartesianGrid strokeDasharray="3 3" vertical={false} />
			<XAxis dataKey="time" />
			<YAxis />
			<Tooltip />
			<Legend />
			<Line type="monotone" dataKey="xPos" stroke="red" />
			<Line type="monotone" dataKey="yPos" stroke="blue" />
			<Line type="monotone" dataKey="zPos" stroke="green" />
			<Line type="monotone" dataKey="yawPos" stroke="orange" />
			</LineChart>
		</ResponsiveContainer>

		{/* PID1 Graph */}
		<ResponsiveContainer width="100%" height={200} style={{ backgroundColor: 'black' }}>
			<LineChart data={chartData}>
			<CartesianGrid strokeDasharray="3 3" vertical={false} />
			<XAxis dataKey="time" />
			<YAxis domain={[-1, 1]} />
			<Tooltip />
			<Legend />
			<Line type="monotone" dataKey="xPID1" stroke="red" />
			<Line type="monotone" dataKey="yPID1" stroke="blue" />
			<Line type="monotone" dataKey="zPID1" stroke="green" />
			<Line type="monotone" dataKey="yawPID2" stroke="orange" />
			</LineChart>
		</ResponsiveContainer>

		{/* PID2 Graph */}
		<ResponsiveContainer width="100%" height={200} style={{ backgroundColor: 'black' }}>
			<LineChart data={chartData}>
			<CartesianGrid strokeDasharray="3 3" vertical={false} />
			<XAxis dataKey="time" />
			<YAxis domain={[-1, 1]} />
			<Tooltip />
			<Legend />
			<Line type="monotone" dataKey="xPID2" stroke="red" />
			<Line type="monotone" dataKey="yPID2" stroke="blue" />
			<Line type="monotone" dataKey="zPID2" stroke="green" />
			</LineChart>
		</ResponsiveContainer>

		{/* PWM Graph */}
		<ResponsiveContainer width="100%" height={200} style={{ backgroundColor: 'black' }}>
			<LineChart data={chartData}>
			<CartesianGrid strokeDasharray="3 3" vertical={false} />
			<XAxis dataKey="time" />
			<YAxis domain={[400, 1600]} />
			<Tooltip />
			<Legend />
			<Line type="monotone" dataKey="xPWM" stroke="red" />
			<Line type="monotone" dataKey="yPWM" stroke="blue" />
			<Line type="monotone" dataKey="zPWM" stroke="green" />
			<Line type="monotone" dataKey="yawPWM" stroke="orange" />
			</LineChart>
		</ResponsiveContainer>
	</div>
	);
}

export { Graph };
