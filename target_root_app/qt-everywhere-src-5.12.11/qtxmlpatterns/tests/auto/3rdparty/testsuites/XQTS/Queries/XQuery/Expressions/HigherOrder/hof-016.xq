xquery version "1.1";
(: Higher Order Functions :)
(: Function expecting a function :)
(: Author - Michael Kay, Saxonica :)

declare function local:case($x as function(*), $y as xs:string) as xs:string {
  $x($y)
};


local:case(upper-case#1, "Mike"), local:case(lower-case#1, "Mike")