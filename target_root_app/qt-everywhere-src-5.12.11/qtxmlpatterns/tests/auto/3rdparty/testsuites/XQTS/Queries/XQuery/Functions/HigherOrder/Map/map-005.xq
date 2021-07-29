xquery version "1.1";
(: Higher Order Functions :)
(: map function :)
(: Author - Michael Kay, Saxonica :)

declare function local:e($n as xs:string) as element() {
  element {$n} {}
};

declare function local:n($e as element()) as xs:string {
  local-name($e)
};

<out>{map(local:n#1, map(local:e#1, ("john", "mary", "jane", "anne", "peter", "ian")))}</out>