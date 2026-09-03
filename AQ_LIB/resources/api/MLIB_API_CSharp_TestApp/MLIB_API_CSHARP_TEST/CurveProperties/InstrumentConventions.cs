using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;

namespace MLIB_API_CSHARP_TEST
{
    public class InstrumentConventions 
    {

    public static String CALENDAR = "calendar";
    public static String DAYCOUNT = "daycount";
    public static String DAYCOUNT_FIX = "daycountfix";
    public static String DAYCOUNT_FLOAT = "daycountfloat";
    public static String FREQUENCY = "frequency";
    public static String FREQUENCY_FIX = "frequencyfix";
    public static String FREQUENCY_FLOAT = "frequencyfloat";
    public static String FREQUENCY_FLOAT_1Y = "frequencyfloat.1y";
    public static String RESET_LAG = "resetlag";
    public static String SLIDING_RULE = "slidingrule";
    public static String IS_EOM_ROLL = "iseomroll";
    public static String EOM_DAY = "eomday";
    public static String GENERATE_METHOD = "generatemethod";
    public static String FIRST_RATE = "firstrate";
    public static String SHORT_TERM_CONVENTION = "shorttermconvention";
    public static String LONG_TERM_CONVENTION = "longtermconvention";
    public static String LONG_TERM = "longterm";
    public static String LONG_TERM_GENERATE_METHOD = "longterm.generatemethod";
    public static String APPLY_TENSION = "ApplyTension";
    public static String TENSION_GAP = "TensionGap";
    public static String INTERPOLATION = "interpolation";
    public static String IS_TIME_INTERPOLATION = "IsTimeInterpolation";
    public static String IS_NEWTON_RAPHSON = "IsNewtonraphson";
    public static String IS_SIMULTANEOUS_EQ = "issimultaneouseq";
    public static String OPTIMIZE_METHOD = "optimizemethod";
    public static String EPSILON = "epsilon";
    public static String GRADIENT_EPSILON = "gradientEpsilon";
    public static String DELTA = "delta";
    public static String MAX_LOOP = "maxLoop";
    private static String MEAN_REVERSION = "meanReversion";

    private Dictionary<String, String> properties;

    private InstrumentConventions(Dictionary<String, String> properties) {
        this.properties = properties;
    }

    public VecVecString toMlib()
    {
        VecVecString ret = new VecVecString();
        foreach (KeyValuePair<string, string> entry in properties)
        {
            VecString vecString = new VecString();
            vecString.Add(entry.Key);
            vecString.Add(entry.Value);

            ret.Add(vecString);
        }

        return ret;
    }

    public class Builder 
    {
        private Dictionary<String, String> builderProperties = new Dictionary<String, String>();

        public InstrumentConventions build()
        {
            return new InstrumentConventions(builderProperties);
        }

        public Builder calendar(Calendar calendar) 
        {
            builderProperties.Add(CALENDAR, calendar.toString());

            return this;
        }

        public Builder daycount(DayCount dayCount) 
        {
            builderProperties.Add(DAYCOUNT, dayCount.getMlibArg());

            return this;
        }

        public Builder dayCountFix(DayCount dayCountFix) 
        {
            builderProperties.Add(DAYCOUNT_FIX, dayCountFix.getMlibArg());

            return this;
        }

        public Builder dayCountFloat(DayCount dayCountFloat) 
        {
            builderProperties.Add(DAYCOUNT_FLOAT, dayCountFloat.getMlibArg());

            return this;
        }

        public Builder frequency(Frequency frequency) 
        {
            builderProperties.Add(FREQUENCY, frequency.getMlibArg());

            return this;
        }

        public Builder frequencyFix(Frequency frequencyFix) 
        {
            builderProperties.Add(FREQUENCY_FIX, frequencyFix.getMlibArg());

            return this;
        }

        public Builder frequencyFloat(Frequency frequencyFloat) 
        {
            builderProperties.Add(FREQUENCY_FLOAT, frequencyFloat.getMlibArg());

            return this;
        }

        public Builder resetLag(String resetLag) 
        {
            builderProperties.Add(RESET_LAG, resetLag);

            return this;
        }

        public Builder slidingrule(SlidingRule slidingRule) 
        {
            builderProperties.Add(SLIDING_RULE, slidingRule.toString());

            return this;
        }

        public Builder isEomRoll(bool isEomRoll) 
        {
            builderProperties.Add(IS_EOM_ROLL, Convert.ToString(isEomRoll));

            return this;
        }

        public Builder eomDay(int eomDay) {
            builderProperties.Add(EOM_DAY, Convert.ToString(eomDay));

            return this;
        }

        public Builder generateMethod(GenerateMethod generateMethod) 
        {
            builderProperties.Add(GENERATE_METHOD, generateMethod.getMlibArg());

            return this;
        }

        public Builder firstRate(FirstRate firstRate) 
        {
            builderProperties.Add(FIRST_RATE, firstRate.getMlibArg());

            return this;
        }

        public Builder shortTermConvention(ShortTermConvention shortTermConvention) 
        {
            builderProperties.Add(SHORT_TERM_CONVENTION, shortTermConvention.getMlibArg());

            return this;
        }

        public Builder longTermConvention(LongTermConvention longTermConvention) 
        {
            builderProperties.Add(LONG_TERM_CONVENTION, longTermConvention.getMlibArg());

            return this;
        }

        public Builder longTerm(String longTerm) {
            builderProperties.Add(LONG_TERM, longTerm);

            return this;
        }

        public Builder longTermGenerateMethod(LongTermGenerateMethod longTermGenerateMethod) 
        {
            builderProperties.Add(LONG_TERM_GENERATE_METHOD, longTermGenerateMethod.getMlibArg());

            return this;
        }

        public Builder applyTension(bool applyTension) {
            builderProperties.Add(APPLY_TENSION, Convert.ToString(applyTension));

            return this;
        }

        public Builder tensionGap(int tensionGap) {
            builderProperties.Add(TENSION_GAP, Convert.ToString(tensionGap));

            return this;
        }

        public Builder interpolation(Interpolation interpolation) 
        {
            builderProperties.Add(INTERPOLATION, interpolation.asMlibArg());

            return this;
        }

        public Builder isNewtonRaphson(bool isNewtonRaphson) 
        {
            builderProperties.Add(IS_NEWTON_RAPHSON, Convert.ToString(isNewtonRaphson));

            return this;
        }

        public Builder isSimultaneousEq(bool isSimultaneousEq) 
        {
            builderProperties.Add(IS_SIMULTANEOUS_EQ, Convert.ToString(isSimultaneousEq));

            return this;
        }

        public Builder optimizemethod(OptimizeMethod optimizemethod) 
        {
            builderProperties.Add(OPTIMIZE_METHOD, optimizemethod.getMlibArg());

            return this;
        }

        public Builder epsilon(double epsilon) 
        {
            builderProperties.Add(EPSILON, Convert.ToString(epsilon));

            return this;
        }

        public Builder gradientEpsilon(double gradientEpsilon) 
        {
            builderProperties.Add(GRADIENT_EPSILON, Convert.ToString(gradientEpsilon));

            return this;
        }

        public Builder delta(double delta) 
        {
            builderProperties.Add(DELTA, Convert.ToString(delta));

            return this;
        }

        public Builder maxLoop(int maxLoop) 
        {
            builderProperties.Add(MAX_LOOP, Convert.ToString(maxLoop));

            return this;
        }

        public Builder meanReversion(double meanReversion) 
        {
            builderProperties.Add(MEAN_REVERSION, Convert.ToString(meanReversion));

            return this;
        }
        
    }
}

}
