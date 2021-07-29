(:**************************************************************:)
(: Test: functx-fn-position-3                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";

let $in-xml := <in-xml>
     <a>1</a>
     <c>2</c>
     <a>3</a>
     <a>4</a>
     <a>5</a>
   </in-xml>
return ($in-xml/a[position() = 3])
