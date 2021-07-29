xquery version "1.1";
(: Higher Order Functions :)
(: Nested inline functions referencing grandfather local variables :)
(: Author - Michael Kay, Saxonica :)

declare function local:splitter($x as xs:string) as (function() as xs:string*)* {
  for $sep in ('\s', ',', '!') return function() { 
    for $i in tokenize($x, $sep) return 
      let $f := function(){ concat($sep, ':', upper-case($i)) }
      return $f()
    
  }
};

<out>{
  for $f as function(*) in local:splitter("How nice! Thank you, I enjoyed that.")
  return <tokens>{
    for $t in $f() return <token>{$t}</token>
  }</tokens>
}</out> 