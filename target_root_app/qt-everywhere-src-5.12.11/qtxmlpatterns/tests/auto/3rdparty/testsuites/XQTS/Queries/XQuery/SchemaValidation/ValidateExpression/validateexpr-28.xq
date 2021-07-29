(: name: validateexpr-28                                         :)
(: description: Test Bug 5223, [XPath] Casting rules in 3.5.2    :)
(:              General Comparisons (editorial)                  :)

(: insert-start :)
import schema namespace t="http://www.example.com/ns/test/xq-e18";
declare variable $input-context external;
(: insert-end :)

let $doc := validate strict {
  document {
    <doc xmlns="http://www.example.com/ns/test/xq-e18">
      <para1>PT3H21M</para1>
      <para2>P3M</para2>
    </doc>
  }
}

let $p1 := data($doc/t:doc/t:para1)
let $p2 := data($doc/t:doc/t:para2)

let $t1 := xs:dayTimeDuration("PT3H21M")
let $t2 := xs:yearMonthDuration("P3M")
return
  <results>
    <case1>{if ($p1 = $t1) then "pass" else "fail"}</case1>
    <case2>{if ($p2 = $t2) then "pass" else "fail"}</case2>
  </results>
