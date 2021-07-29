(: Name: annex-7 :)
(: Description: User defined function # 7 from annex E of F& O Specs. :)

declare namespace eg = "http://example.org";

declare function eg:string-pad (
  $padString as xs:string?,
  $padCount as xs:integer) as xs:string 
{
   fn:string-join((for $i in 1 to $padCount return $padString), "")
};

let $arg1 as xs:string := "A String"
let $arg2 as xs:integer := 3
return
 eg:string-pad($arg1,$arg2)