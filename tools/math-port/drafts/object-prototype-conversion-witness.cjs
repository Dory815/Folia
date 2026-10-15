const trace = [];
const prototypeBrands = [Object.prototype.toString.call(Number.prototype), Object.prototype.toString.call(Boolean.prototype), Object.prototype.toString.call(String.prototype)];
const plain = [{}].join(',');
const plainSeparator = [1, 2].join({});
const nullishPair = [null, undefined].join({});
const separator = { [Symbol.toPrimitive](hint) { trace.push(`separator:${hint}`); return '|'; } };
const value = { get toString() { trace.push('get:toString'); return function () { trace.push('call:toString'); return 'v'; }; } };
const converted = [value].join(separator);
const tagged = { get [Symbol.toStringTag]() { trace.push('get:tag'); return 'Widget'; } };
let badResult;
try { [{ toString() { return {}; } }].join(); } catch (error) { badResult = error.name; }
const symbolTagDescriptor = Object.getOwnPropertyDescriptor(Symbol.prototype, Symbol.toStringTag);
const defaultSymbolBrand = Object.prototype.toString.call(Symbol('x'));
const boxedSymbol = Object(Symbol('x'));
delete Symbol.prototype[Symbol.toStringTag];
const deletedSymbolBrand = Object.prototype.toString.call(boxedSymbol);
Object.defineProperty(Symbol.prototype, Symbol.toStringTag, { value: 7, writable: false, enumerable: false, configurable: true });
const nonStringSymbolBrand = Object.prototype.toString.call(boxedSymbol);
Object.defineProperty(Symbol.prototype, Symbol.toStringTag, symbolTagDescriptor);
const ownTagTrace = [];
Object.defineProperty(boxedSymbol, Symbol.toStringTag, { get() { ownTagTrace.push('own-tag'); return 'OwnSymbol'; } });
const ownSymbolBrand = Object.prototype.toString.call(boxedSymbol);
const primitiveTrace = [];
Object.defineProperty(String.prototype, Symbol.toStringTag, { configurable: true, get() { primitiveTrace.push('string-tag'); return 'TaggedString'; } });
Object.defineProperty(Symbol.prototype, Symbol.toStringTag, { configurable: true, get() { primitiveTrace.push('symbol-tag'); return 'TaggedSymbol'; } });
const primitiveBrands = [Object.prototype.toString.call('ab'), Object.prototype.toString.call(Symbol('x'))];
const boxedString = Object.prototype.valueOf.call('ab');
const boxedStringShape = [boxedString.length, boxedString[0], boxedString[1], Object.keys(boxedString)];
const brands = [Object.prototype.toString.call({}), Object.prototype.toString.call([]),
  Object.prototype.toString.call(function () {}), Object.prototype.toString.call(tagged)];
const actual = { plain, plainSeparator, nullishPair, converted, brands, prototypeBrands, primitiveBrands, primitiveTrace,
  boxedStringShape, badResult, trace, symbolTagDescriptor: [symbolTagDescriptor.value, symbolTagDescriptor.writable,
  symbolTagDescriptor.enumerable, symbolTagDescriptor.configurable], defaultSymbolBrand, deletedSymbolBrand,
  nonStringSymbolBrand, ownSymbolBrand, ownTagTrace };
const expected = {
  plain: '[object Object]', plainSeparator: '1[object Object]2', nullishPair: '[object Object]', converted: 'v',
  brands: ['[object Object]', '[object Array]', '[object Function]', '[object Widget]'],
  prototypeBrands: ['[object Number]', '[object Boolean]', '[object String]'],
  primitiveBrands: ['[object TaggedString]', '[object TaggedSymbol]'], primitiveTrace: ['string-tag', 'symbol-tag'],
  boxedStringShape: [2, 'a', 'b', ['0', '1']], badResult: 'TypeError',
  trace: ['separator:string', 'get:toString', 'call:toString', 'get:tag'],
  symbolTagDescriptor: ['Symbol', false, false, true], defaultSymbolBrand: '[object Symbol]',
  deletedSymbolBrand: '[object Object]', nonStringSymbolBrand: '[object Object]', ownSymbolBrand: '[object OwnSymbol]',
  ownTagTrace: ['own-tag']
};
if (JSON.stringify(actual) !== JSON.stringify(expected)) throw new Error(JSON.stringify(actual));
console.log(JSON.stringify(actual));
