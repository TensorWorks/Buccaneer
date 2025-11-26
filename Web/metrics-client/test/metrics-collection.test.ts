import { MetricsCollection, formatMetricName } from '../src/metrics-collection.js';

describe('formatMetricName', () => {
    it('should replace dashes with underscores', () => {
        expect(formatMetricName('my-metric')).toBe('my_metric');
    });

    it('should replace dots with underscores', () => {
        expect(formatMetricName('my.metric')).toBe('my_metric');
    });

    it('should replace spaces with underscores', () => {
        expect(formatMetricName('my metric')).toBe('my_metric');
    });

    it('should replace forward slashes with underscores', () => {
        expect(formatMetricName('my/metric')).toBe('my_metric');
    });

    it('should replace backslashes with underscores', () => {
        expect(formatMetricName('my\\metric')).toBe('my_metric');
    });

    it('should remove parentheses', () => {
        expect(formatMetricName('my(metric)')).toBe('mymetric');
    });

    it('should remove quotes', () => {
        expect(formatMetricName('my"metric"')).toBe('mymetric');
        expect(formatMetricName("my'metric'")).toBe('mymetric');
    });

    it('should remove angle brackets', () => {
        expect(formatMetricName('my<metric>')).toBe('mymetric');
    });

    it('should handle complex names', () => {
        expect(formatMetricName('Frame Time (ms) - Avg./Max')).toBe('Frame_Time_ms___Avg__Max');
    });
});

describe('MetricsCollection', () => {
    describe('constructor', () => {
        it('should create a collection with id', () => {
            const mc = new MetricsCollection('test-id');
            expect(mc.getId()).toBe('test-id');
        });

        it('should create a collection with metadata', () => {
            const mc = new MetricsCollection('test-id', { foo: 'bar', baz: 'qux' });
            const metadata = mc.getMetadata();
            expect(metadata.foo).toBe('bar');
            expect(metadata.baz).toBe('qux');
        });
    });

    describe('storeStat', () => {
        it('should add a single metric', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('fps', 60);
            
            const json = mc.toJSON();
            expect(json.metrics.fps).toBeDefined();
            expect(json.metrics.fps.value).toBe(60);
        });

        it('should format metric names', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('frame-time', 16.67);
            
            const json = mc.toJSON();
            expect(json.metrics.frame_time).toBeDefined();
            expect(json.metrics.frame_time.value).toBe(16.67);
        });

        it('should use custom description', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('fps', 60, 'Frames Per Second');
            
            const json = mc.toJSON();
            expect(json.metrics.fps.description).toBe('Frames Per Second');
        });

        it('should default description to stat name', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('fps', 60);
            
            const json = mc.toJSON();
            expect(json.metrics.fps.description).toBe('fps');
        });

        it('should update existing metrics', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('fps', 60);
            mc.storeStat('fps', 120);
            
            const json = mc.toJSON();
            expect(json.metrics.fps.value).toBe(120);
        });
    });

    describe('storeStatByGroup', () => {
        it('should add a grouped metric', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStatByGroup('player0', 'score', 100);
            
            const json = mc.toJSON();
            expect(json.metrics.score).toBeDefined();
            expect(Array.isArray(json.metrics.score.value)).toBe(true);
            const groupedMetric = json.metrics.score as { description: string; value: Array<Record<string, number>> };
            expect(groupedMetric.value[0].player0).toBe(100);
        });

        it('should add multiple groups to same metric', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStatByGroup('player0', 'score', 100);
            mc.storeStatByGroup('player1', 'score', 200);
            
            const json = mc.toJSON();
            const groupedMetric = json.metrics.score as { description: string; value: Array<Record<string, number>> };
            expect(groupedMetric.value.length).toBe(2);
            expect(groupedMetric.value[0].player0).toBe(100);
            expect(groupedMetric.value[1].player1).toBe(200);
        });

        it('should update existing group value', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStatByGroup('player0', 'score', 100);
            mc.storeStatByGroup('player0', 'score', 150);
            
            const json = mc.toJSON();
            const groupedMetric = json.metrics.score as { description: string; value: Array<Record<string, number>> };
            expect(groupedMetric.value.length).toBe(1);
            expect(groupedMetric.value[0].player0).toBe(150);
        });

        it('should use custom description', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStatByGroup('player0', 'score', 100, 'Player Score');
            
            const json = mc.toJSON();
            expect(json.metrics.score.description).toBe('Player Score');
        });

        it('should format metric names', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStatByGroup('player0', 'ping-time', 50);
            
            const json = mc.toJSON();
            expect(json.metrics.ping_time).toBeDefined();
        });
    });

    describe('setMetadata', () => {
        it('should add metadata', () => {
            const mc = new MetricsCollection('test-id');
            mc.setMetadata('version', '1.0.0');
            
            const metadata = mc.getMetadata();
            expect(metadata.version).toBe('1.0.0');
        });

        it('should update existing metadata', () => {
            const mc = new MetricsCollection('test-id', { version: '1.0.0' });
            mc.setMetadata('version', '2.0.0');
            
            const metadata = mc.getMetadata();
            expect(metadata.version).toBe('2.0.0');
        });
    });

    describe('toJSON', () => {
        it('should include id and timestamp', () => {
            const mc = new MetricsCollection('test-id');
            const json = mc.toJSON();
            
            expect(json.id).toBe('test-id');
            expect(typeof json.timestamp).toBe('number');
            expect(json.timestamp).toBeGreaterThan(0);
        });

        it('should include metadata if present', () => {
            const mc = new MetricsCollection('test-id', { foo: 'bar' });
            const json = mc.toJSON();
            
            expect(json.metadata).toBeDefined();
            expect(json.metadata!.foo).toBe('bar');
        });

        it('should omit metadata if empty', () => {
            const mc = new MetricsCollection('test-id');
            const json = mc.toJSON();
            
            expect(json.metadata).toBeUndefined();
        });

        it('should include all metrics', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('fps', 60);
            mc.storeStatByGroup('player0', 'score', 100);
            
            const json = mc.toJSON();
            expect(json.metrics.fps).toBeDefined();
            expect(json.metrics.score).toBeDefined();
        });
    });

    describe('clear', () => {
        it('should remove all metrics', () => {
            const mc = new MetricsCollection('test-id');
            mc.storeStat('fps', 60);
            mc.storeStatByGroup('player0', 'score', 100);
            
            mc.clear();
            
            const json = mc.toJSON();
            expect(Object.keys(json.metrics).length).toBe(0);
        });

        it('should keep metadata', () => {
            const mc = new MetricsCollection('test-id', { foo: 'bar' });
            mc.storeStat('fps', 60);
            
            mc.clear();
            
            const json = mc.toJSON();
            expect(json.metadata).toBeDefined();
            expect(json.metadata!.foo).toBe('bar');
        });
    });
});
