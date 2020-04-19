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

/*
 * Main class of the TFICF MapReduce implementation.
 * Author: Tyler Stocksdale
 * Date:   10/18/2017
 */
public class TFICF {

    public static void main(String[] args) throws Exception {
        // Check for correct usage
       Configuration conf0 = new Configuration();
		Configuration conf1 = new Configuration();
		int ret = 0;
		
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
		 Job job = Job.getInstance(conf, "word count");
    job.setJarByClass(TFICF.class);
    job.setMapperClass(TokenizerMapper.class);
    job.setCombinerClass(IntSumReducer.class);
    job.setReducerClass(IntSumReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(IntWritable.class);
    FileInputFormat.addInputPath(job, wcInputPath);
    FileOutputFormat.setOutputPath(job, wcOutputPath);
    System.exit(job.waitForCompletion(true) ? 0 : 1);
		
		return 1;
    }
	public static class TokenizerMapper
       extends Mapper<Object, Text, Text, IntWritable>{

    private final static IntWritable one = new IntWritable(1);
    private Text word = new Text();

    public void map(Object key, Text value, Context context
                    ) throws IOException, InterruptedException {
      StringTokenizer itr = new StringTokenizer(value.toString());
      while (itr.hasMoreTokens()) {
        word.set(itr.nextToken());
        context.write(word, one);
      }
    }
  }

  public static class IntSumReducer
       extends Reducer<Text,IntWritable,Text,IntWritable> {
    private IntWritable result = new IntWritable();

    public void reduce(Text key, Iterable<IntWritable> values,
                       Context context
                       ) throws IOException, InterruptedException {
      int sum = 0;
      for (IntWritable val : values) {
        sum += val.get();
      }
      result.set(sum);
      context.write(key, result);
    }
  }
	
}

