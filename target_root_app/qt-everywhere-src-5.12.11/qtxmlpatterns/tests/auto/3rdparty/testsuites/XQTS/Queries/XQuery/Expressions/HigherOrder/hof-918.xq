xquery version "1.1";
(: Higher Order Functions :)
(: partial-apply supplying an incorrect function parameter :)
(: Author - Michael Kay, Saxonica :)

declare function local:round($x as xs:double, $algorithm as (function(xs:double) as xs:double)) as xs:double {
  $algorithm($x)
};

declare variable $roundToCeiling := partial-apply(local:round#2, upper-case#1, 2);

$roundToCeiling(12.4)