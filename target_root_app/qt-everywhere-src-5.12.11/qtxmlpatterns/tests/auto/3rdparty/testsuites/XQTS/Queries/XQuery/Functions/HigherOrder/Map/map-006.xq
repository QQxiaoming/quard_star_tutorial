xquery version "1.1";
(: Higher Order Functions :)
(: map function :)
(: Author - Michael Kay, Saxonica :)

declare function local:children($n as node()) as node() {
  $n/child::node()
};

declare function local:attributes($e as node()) as node() {
  $e/attribute::node()
};

declare function local:self($e as node()) as node() {
  $e
};

declare function local:union($f as function(node()) as node()*,  $g as function(node()) as node()*)
  as function(node()) as node()* {
  function($a) {$f($a) | $g($a)}
};

let $data := <a aid="a"><b bid="b"><b1/></b><c cid ="c"><c1/></c></a>
let $f := local:union(local:children#1, local:union(local:attributes#1, local:self#1))
return

<out>{map($f, $data/*)[not(. instance of attribute())]}</out>