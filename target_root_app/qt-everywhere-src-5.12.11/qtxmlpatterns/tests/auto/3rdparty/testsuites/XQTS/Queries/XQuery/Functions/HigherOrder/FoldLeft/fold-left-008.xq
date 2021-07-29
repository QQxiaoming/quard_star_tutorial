xquery version "1.1";

(: fold-left-008 :)
(: author Michael Kay, Saxonica :)
(: get the employees who worked the maximum number of hours :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

declare function local:hours($emp as element(employee)) as xs:integer {
  sum($emp/hours/xs:integer(.))
};

declare function local:highest($f as function(item()) as xs:anyAtomicType, $seq as item()*) {
    fold-left(
      function($highestSoFar as item()*, $this as item()*) as item()* {
        if (empty($highestSoFar))
        then $this
        else 
          let $thisValue := $f($this)
          let $highestValue := $f($highestSoFar[1])
          return
            if ($thisValue gt $highestValue)
            then $this
            else if ($thisValue eq $highestValue)
            then ($highestSoFar, $this)
            else $highestSoFar
      }, (), $seq)
};  


let $doc as document-node(element(works)) := $input-context return

<hardest-workers>{
  local:highest(local:hours#1, $doc/works/employee)
}</hardest-workers>  
  
     