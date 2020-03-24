import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import java.io.IOException;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
/*
 * Main class of the TFICF MapReduce implementation.
 * Author: Tyler Stocksdale
 * Date:   10/18/2017
 */
public class TFICF {

    public static void main(String[] args) throws Exception {
        // Check for correct usage
        if (args.length != 2) {
            System.err.println("Usage: TFICF <input corpus0 dir> <input corpus1 dir>");
            System.exit(1);
        }
		
		// return value of run func
		int ret = 0;
		
		// Create configuration
		Configuration conf0 = new Configuration();
		Configuration conf1 = new Configuration();
		
		// Input and output paths for each job
		Path inputPath0 = new Path(args[0]);
		Path inputPath1 = new Path(args[1]);
        try{
            ret = run(conf0, inputPath0, 0);
        }catch(Exception e){
            e.printStackTrace();
        }
        if(ret == 0){
        	try{
            	run(conf1, inputPath1, 1);
        	}catch(Exception e){
            	e.printStackTrace();
        	}        	
        }
     
     	System.exit(ret);
    }
		
	public static int run(Configuration conf, Path path, int index) throws Exception{
		// Input and output paths for each job

		Path wcInputPath = path;
		Path wcOutputPath = new Path("output" +index + "/WordCount");
		Path dsInputPath = wcOutputPath;
		Path dsOutputPath = new Path("output" + index + "/DocSize");
		Path tficfInputPath = dsOutputPath;
		Path tficfOutputPath = new Path("output" + index + "/TFICF");
		
		// Get/set the number of documents (to be used in the TFICF MapReduce job)
        	FileSystem fs = path.getFileSystem(conf);
        	FileStatus[] stat = fs.listStatus(path);
		String numDocs = String.valueOf(stat.length);
		conf.set("numDocs", numDocs);
		
		// Delete output paths if they exist
		FileSystem hdfs = FileSystem.get(conf);
		if (hdfs.exists(wcOutputPath))
			hdfs.delete(wcOutputPath, true);
		if (hdfs.exists(dsOutputPath))
			hdfs.delete(dsOutputPath, true);
		if (hdfs.exists(tficfOutputPath))
			hdfs.delete(tficfOutputPath, true);
		
		// Create and execute Word Count job
		
		/************ YOUR CODE HERE ************/
		Job wcJob = Job.getInstance(conf, "WordCount");

		// tell mapreduce what to look for in jar
		wcJob.setJarByClass(TFICF.class);

		// Set mapper and reducer classes
		wcJob.setMapperClass(WCMapper.class);
		wcJob.setCombinerClass(WCReducer.class);
		wcJob.setReducerClass(WCReducer.class);
		//output key value data type

		wcJob.setOutputKeyClass(Text.class);
		wcJob.setOutputValueClass(IntWritable.class);
		// Set Input and output paths
		FileInputFormat.addInputPath(wcJob, wcInputPath);
		FileOutputFormat.setOutputPath(wcJob, wcOutputPath);
//		wcJob.waitForCompletion(true);
			
		// Create and execute Document Size job
		
			/************ YOUR CODE HERE ************/
/*		Job dsJob = Job.getInstance(conf, "DocSize");
*/
		// tell mapreduce what to look for in jar
/*		dsJob.setJarByClass(TFICF.class);

*/
		/*//set file input format
		dsJob.setInputFormatClass(TextInputFormat.class);
		dsJob.setOutputFormatClass(TextOutputFormat.class);
*/


		// Set mapper and reducer classes
/*		dsJob.setMapperClass(DSMapper.class);
               // dsJob.setCombinerClass(DSReducer.class);
		dsJob.setReducerClass(DSReducer.class);
		//output key value data type

		dsJob.setOutputKeyClass(Text.class);
		dsJob.setOutputValueClass(Text.class);
		// Set Input and output paths
		FileInputFormat.addInputPath(dsJob, dsInputPath);
		FileOutputFormat.setOutputPath(dsJob, dsOutputPath);
		//dsJob.waitForCompletion(true);


		//Create and execute TFICF job
		
			/************ YOUR CODE HERE ************/
/*		Job TFICFJob = Job.getInstance(conf, "DocSize");
		// Set Input and output paths
		FileInputFormat.addInputPath(TFICFJob, tficfInputPath);
		FileOutputFormat.setOutputPath(TFICFJob, tficfOutputPath);
		// tell mapreduce what to look for in jar
		TFICFJob.setJarByClass(TFICF.class);


		//set file input format
		TFICFJob.setInputFormatClass(TextInputFormat.class);
		TFICFJob.setOutputFormatClass(TextOutputFormat.class);

		//output key value data type

		TFICFJob.setOutputKeyClass(Text.class);
		TFICFJob.setOutputValueClass(IntWritable.class);

		// Set mapper and reducer classes
		TFICFJob.setMapperClass(TFICFMapper.class);
		TFICFJob.setReducerClass(TFICFReducer.class);
		TFICFJob.waitForCompletion(true);

		//Return final job code , e.g. retrun tficfJob.waitForCompletion(true) ? 0 : 1
		*/
			/************ YOUR CODE HERE ************/
		// Return status to main
	return wcJob.waitForCompletion(true) ? 0 : 1;

    }
	
	/*
	 * Creates a (key,value) pair for every word in the document 
	 *
	 * Input:  ( byte offset , contents of one line )
	 * Output: ( (word@document) , 1 )
	 *
	 * word = an individual word in the document
	 * document = the filename of the document
	 */
	public static class WCMapper extends Mapper<Object, Text, Text, IntWritable> {
		
		/************ YOUR CODE HERE ************/

		String str="";
		public void map(Object key, Text value, Context context) throws IOException, InterruptedException {

			//ITERATE OVER ALL LINES
			StringTokenizer line = new StringTokenizer(value.toString());
			while (line.hasMoreTokens()) {
				//getting input file name
				String fileName = ((FileSplit) context.getInputSplit()).getPath().getName();
				String parse_word=line.nextToken();
				if(!Pattern.matches("(^[a-zA-Z'()\\[\\]\"].*)",parse_word))
					continue;
				System.out.print("****************************" +parse_word);
				String word=parse_word.replaceAll("[^a-zA-Z-\u00C0-\u017F\\]\\[]*","");
				if(word.isEmpty() || word.equals("-"))
					continue;
				System.out.println("  " +word);
				str=word+"@"+fileName;
				//writing to the output file
				context.write(new Text(str.toLowerCase()), new IntWritable(1));
			}
		}
		
    }

    /*
	 * For each identical key (word@document), reduces the values (1) into a sum (wordCount)
	 *
	 * Input:  ( (word@document) , 1 )
	 * Output: ( (word@document) , wordCount )
	 *
	 * wordCount = number of times word appears in document
	 */
	public static class WCReducer extends Reducer<Text, IntWritable, Text, IntWritable> {
		
		/************ YOUR CODE HERE ************/
		public void reduce(Text key, Iterable<IntWritable> values, Context context) throws IOException, InterruptedException
		{
			int sum=0;
			for(IntWritable value:values)
			{
				sum+=value.get();
			}
			context.write(key,new IntWritable(sum));
		}
		
    }
    /*
     * Rearranges the (key,value) pairs to have only the document as the k* Input:  ( (word@document) , wordCount )
     * Input:  ( (word@document) , wordCount )
     * Output: ( document , (word=wordCount) )
     */	
    public static class DSMapper extends Mapper<Object, Text , Text, Text> {
		
		/************ YOUR CODE HERE ************/
	public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
		//ITERATE OVER ALL LINES
		StringTokenizer line = new StringTokenizer(value.toString());
			while (line.hasMoreTokens()) {
				String lineNext=line.nextToken();
				//System.out.println(lineNext);
				String doc=lineNext.split("@")[1];
				String word=lineNext.split("@")[0]+"="+line.nextToken();
				//System.out.println(doc+"doc:"+word+"word:");
				context.write(new Text(doc),new Text(word));
			}
		}

    }
    /*
     * For each identical key (document), reduces the values (word=wordCount) into a sum (docSize) 
     *
     * Input:  ( document , (word=wordCount) )
     * Output: ( (word@document) , (wordCount/docSize) )
     *
     * docSize = total number of words in the document
     */	
     public static class DSReducer extends Reducer<Text, Text, Text, Text> {
		
		/************ YOUR CODE HERE ************/
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
			int docSize = 0;
			Iterator values1=values.iterator();
			ArrayList<String> doc=new ArrayList<>();
			ArrayList<String> key1=new ArrayList<>();
			while ( values1.hasNext()) {
				docSize += Integer.valueOf((values1.next().toString()).split("=")[1]);
				String value_str=values1.next().toString();
		//		System.out.println("+++++++++++++++++"+value_str);
				doc.add(value_str.split("=")[1]+"/"+String.valueOf(docSize));
				key1.add(value_str.split("=")[0]+"@"+key.toString());
			}
			System.out.println("+++++++++++++++++"+docSize+"key = "+ key.toString());
			//values1=values.iterator();
			//System.out.println(values1.hasNext()+"++++++++++");
			for(int i=0;i<doc.size();i++) {
				//System.out.println("---- "+values1.next().toString());
				//String key1 = (values1.next().toString()).split("=")[0]+"@"+key.toString();
				//System.out.println("key= "+key1);
				//String val= (values1.next().toString()).split("=")[1]+"/"+String.valueOf(docSize);
				//System.out.println("val= "+val);
				context.write(new Text(key1.get(i)),new Text(doc.get(i)));


			}

		}
    }
}
