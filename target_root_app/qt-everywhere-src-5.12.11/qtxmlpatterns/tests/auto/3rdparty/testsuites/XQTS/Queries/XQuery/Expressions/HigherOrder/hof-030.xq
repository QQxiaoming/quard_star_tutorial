xquery version "1.1";
(: Higher Order Functions :)
(: Return an inline function that uses global variables :)
(: Author - Michael Kay, Saxonica :)

declare variable $sep as xs:string external := "\s";

declare function local:splitter() as (function(xs:string) as xs:string*)? {
  function($x as xs:string) { for $i in tokenize($x, $sep) return upper-case($i)}
};

string-join(local:splitter()("A nice cup of tea"), '|')