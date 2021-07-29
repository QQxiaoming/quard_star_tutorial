(:**************************************************************:)
(: Test: functx-fn-distinct-values-4                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <in-xml>
  <a>3</a>
  <b>5</b>
  <b>3</b>
</in-xml>
return (distinct-values(
   (xs:integer('1'),
    xs:decimal('1.0'),
    xs:float('1.0E0') ) ))
