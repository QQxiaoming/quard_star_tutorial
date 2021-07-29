xquery version "1.1";

(: filter-005 :)
(: author Michael Kay, Saxonica :)
(: implement eg:index-of-node() :)


declare function local:index-of-node($seqParam as node()*, $srchParam as node()) as xs:integer* {
    filter(
      function($this as xs:integer) as xs:boolean {$seqParam[$this] is $srchParam}, 
      1 to count($seqParam)
    )
};  


let $nodes := (<a/>, <b/>, <c/>, <d/>, <e/>, <f/>)
let $perm := ($nodes[1], $nodes[2], $nodes[3], $nodes[1], $nodes[2], $nodes[4], $nodes[2], $nodes[1]) return

<indexes>{
  local:index-of-node($perm, $nodes[2])
}</indexes>  
  
     