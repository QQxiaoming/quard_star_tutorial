(:**************************************************************:)
(: Test: functx-fn-distinct-values-1                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <in-xml>
  <a>3</a>
  <b>5</b>
  <b>3</b>
</in-xml>
return (distinct-values( ('a', 'b', 'a') ))
