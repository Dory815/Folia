export const DIRECTION = { None: '', Vertical: 'v', Horizontal: 'h' };
export const NOSTRETCH = { dir: DIRECTION.None };
export const TEXCLASS = { OPEN: 4, CLOSE: 5 };
export class CharOptions {}
export class VariantData {}
export class DelimiterData {}
export class FontData {}
export class FontDataClass {}
export class CommonOutputJax {}
export class CommonWrapperConstructor {}
export class MmlNode {}
export class MmlMo {}
export class CommonWrapperFactory {}
export class CommonWrapperClass {}
export class BBox {
  constructor({w=0,h=-1e6,d=-1e6}={}) { Object.assign(this,{w,h,d,R:0,L:0,ic:0,sk:0,dx:0}); }
  static empty() { return new BBox(); }
  static zero() { return new BBox({w:0,h:0,d:0}); }
}
export class LineBBox extends BBox {
  static from(box, leading) { const out=Object.assign(new LineBBox(),box); out.lineLeading=leading; return out; }
  getIndentData() { this.indentRead=true; }
}
export function unicodeChars(text) { return Array.from(text, c => c.codePointAt(0)); }
export class CommonWrapper {
  constructor(factory,node,parent=null) {
    this.factory=factory; this.node=node; this.parent=parent; this.jax=factory.jax;
    this.font=factory.font; this.stretch=NOSTRETCH; this.childNodes=[];
    this.bbox=new BBox({w:node.w??0,h:node.h??0,d:node.d??0});
    this.linebreakOptions={lineleading:.2}; this.multChar=null;
  }
  computeBBox(box) { box.w=this.node.w??0; box.h=this.node.h??0; box.d=this.node.d??0; }
  copySkewIC(box) { box.sk=this.node.sk??0; box.ic=this.node.ic??0; }
  getText() { return this.node.getText(); }
  getOuterBBox() { return this.node.outerBBox??new BBox({w:this.bbox.w,h:this.bbox.h,d:this.bbox.d}); }
  length2em(value, fallback, scale) {
    if (value == null || value === '') return fallback;
    const number=typeof value==='number'?value:Number.parseFloat(value);
    return Number.isFinite(number)?number*(scale??1):fallback;
  }
  createMo(text) { return this.factory.createMo(text); }
  invalidateBBox() { this.invalidated=true; }
  getVariant() { this.variant='normal'; }
}
