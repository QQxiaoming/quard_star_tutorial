xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-010                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use quotes in an EQName                       :)
(:*******************************************************:)


<out>{
  (<a xmlns:ex="http://www.example.com/ns?p='23'"><ex:b>93.7</ex:b></a>)
     /'http://www.example.com/ns?p=''23''':b
}</out>
  