import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import javafx.application.Application;
import javafx.scene.control.Button;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.stage.Stage;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.scene.control.TextArea;


public class Benchmark extends Application {
	
	public TextArea text = new TextArea();
	
	public void start(final Stage stage) throws Exception {
		
		Button benchmark = new Button("Start Benchmark");
		benchmark.setLayoutX(150);
		benchmark.setLayoutY(10);
		
		Button clear = new Button("Clear");
		clear.setLayoutX(400);
		clear.setLayoutY(10);
		
		text.setLayoutX(60);
		text.setLayoutY(50);
		text.setPrefRowCount(50);
        text.setPrefColumnCount(100);
        text.setWrapText(true);
        text.setPrefWidth(600);
        text.setEditable(false);
        
        Group group = new Group(benchmark, clear, text);
        
        final Scene scene = new Scene(group, 700, 900);
        
        benchmark.setOnAction(new EventHandler<ActionEvent>() {
			
			public void handle(ActionEvent e) {
				
				String s;
				
				File file = new File("D:\\UTCN\\An 3\\Semestrul 2\\SCS\\Benchmark\\result.txt");
				ProcessBuilder p = new ProcessBuilder().inheritIO();
		        System.out.println("Started EXE");
		        p.command("D:\\UTCN\\An 3\\Semestrul 2\\SCS\\cuda\\x64\\Debug\\cuda.exe");   

		        try {
					Process process = p.start();
					p.redirectOutput(file);
					process.waitFor();
				} catch (IOException e1) {
					e1.printStackTrace();
				} catch (InterruptedException e1) {
					e1.printStackTrace();
				}
		        
		        try {
					BufferedReader br = new BufferedReader(new FileReader(file));
					while ((s = br.readLine()) != null) {
						text.appendText(s + "\n");
					}
					br.close();
				} catch (FileNotFoundException e1) {
					e1.printStackTrace();
				} catch (IOException e1) {
					e1.printStackTrace();
				}
			}
		});
        
        clear.setOnAction(new EventHandler<ActionEvent>() {
        	
        	public void handle(ActionEvent e) {
        		
        		text.clear();
        	}
        });
        
        stage.setTitle("Benchmark");
		stage.setScene(scene);
		stage.show();
	}
	
	public static void main(String[] args) {
		launch(args);
	}
}


