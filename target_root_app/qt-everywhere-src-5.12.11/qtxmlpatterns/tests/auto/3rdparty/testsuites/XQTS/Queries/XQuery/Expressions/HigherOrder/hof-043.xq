xquery version "1.1";
(: Higher Order Functions :)
(: untypedAtomic conversion works for various kinds of functions :)
(: Author - Michael Kay, Saxonica :)

declare function local:round($x as xs:double) as xs:double {
  fn:floor($x)
};

declare function local:ops() as (function(xs:double) as xs:double)* {
  (abs#1, local:round#1, function($x){$x+1}, partial-apply(round-half-to-even#2, 2, 2))
};


string-join(for $f in local:ops() return string($f(xs:untypedAtomic('123.456'))), '~')