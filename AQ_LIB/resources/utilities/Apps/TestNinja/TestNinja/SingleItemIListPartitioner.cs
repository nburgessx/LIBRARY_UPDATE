using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Threading;

/* @brief   This class is a simple example of a partitioner which performs
 *          load-balancing across a number of threads.
 *        
 *          Inspired by: http://www.drdobbs.com/windows/custom-parallel-partitioning-with-net-4/224600406?pgno=2
 * 
 * @author  Ian Castleton
 * @date    15 Dec 2016
 * 
 */

namespace TestNinja
{
    public sealed class SingleItemIListPartitioner<T> : OrderablePartitioner<T>
    {
        private readonly IList<T> _source;

        public SingleItemIListPartitioner(IList<T> source)
            : base(keysOrderedInEachPartition: true,
                   keysOrderedAcrossPartitions: false,
                   keysNormalized: true)
        {
            if (source == null) throw new ArgumentNullException("source");
            _source = source;
        }

        public override bool SupportsDynamicPartitions { get { return true; } }

        public override IList<IEnumerator<KeyValuePair<long, T>>>
            GetOrderablePartitions(int partitionCount)
        {
            if (partitionCount < 1)
                throw new ArgumentOutOfRangeException("partitionCount");
            var dynamicPartitioner = GetOrderableDynamicPartitions();
            return (from i in Enumerable.Range(0, partitionCount)
                    select dynamicPartitioner.GetEnumerator()).ToList();
        }

        public override IEnumerable<KeyValuePair<long, T>>
            GetOrderableDynamicPartitions()
        {
            return GetOrderableDynamicPartitionsCore(
                _source, new StrongBox<Int32>(0));
        }

        private static IEnumerable<KeyValuePair<long, T>>
            GetOrderableDynamicPartitionsCore(
                IList<T> source, StrongBox<Int32> nextIteration)
        {
            while (true)
            {
                var iteration =
                    Interlocked.Increment(ref nextIteration.Value) - 1;
                if (iteration >= 0 && iteration < source.Count)
                    yield return new KeyValuePair<long, T>(iteration, source[iteration]);
                else yield break;
            }
        }
    }
}
