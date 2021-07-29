(:**************************************************************:)
(: Test: functx-fn-resolve-QName-all                                  :)
(: Written by: Priscilla Walmsley (Frans Englich is maintainer) :)
(: Date: 2008-05-16+02:00                                       :)
(:**************************************************************:)

declare namespace functx = "http://www.example.com/";
declare namespace ord = "http://datypic.com/ord";
declare namespace dty = "http://datypic.com";
declare namespace dty2 = "http://datypic.com/ns2";

let $root := <root>
  <order xmlns:ord="http://datypic.com/ord"
         xmlns="http://datypic.com">
   <!-- ... -->
  </order>
</root>

return (resolve-QName('myName', $root), resolve-QName('myName', $root/dty:order), resolve-QName(
   'ord:myName', $root/dty:order))