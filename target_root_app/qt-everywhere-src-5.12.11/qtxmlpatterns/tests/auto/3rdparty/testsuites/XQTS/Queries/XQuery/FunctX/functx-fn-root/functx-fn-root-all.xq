(:**************************************************************:)
(: Test: functx-fn-root-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


let $in-xml := <a><x>123</x></a>
return (root(
   doc($input-context1)
   //item[1]), root($in-xml/x))