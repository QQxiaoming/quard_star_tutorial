xquery version "1.1";

(: fold-left-009 :)
(: author Michael Kay, Saxonica :)
(: implement eg:distinct-nodes-stable() :)


declare function local:distinct-nodes-stable($seq as node()*) {
    fold-left(
      function($foundSoFar as node()*, $this as node()) as node()* {
        if ($foundSoFar intersect $this)
        then $foundSoFar
        else ($foundSoFar, $this)
      }, (), $seq)
};  


let $nodes := (<a/>, <b/>, <c/>, <d/>, <e/>, <f/>)
let $perm := ($nodes[1], $nodes[2], $nodes[3], $nodes[1], $nodes[2], $nodes[4], $nodes[2], $nodes[1]) return

<distinct-nodes>{
  local:distinct-nodes-stable($perm)
}</distinct-nodes>  
  
     