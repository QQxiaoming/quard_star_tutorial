xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-901                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Invalid EQName - doubled colon                :)
(:*******************************************************:)


<out>{
  (<a xmlns:ex="http://www.example.com/ns?p='23'"><ex:b>93.7</ex:b></a>)
     /'http://www.example.com/ns?p=''23'''::b
}</out>
  