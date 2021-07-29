xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-902                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Invalid EQName - mismatched quotes            :)
(:*******************************************************:)


<out>{
  (<a xmlns:ex="http://www.example.com/ns?p='23'"><ex:b>93.7</ex:b></a>)
     /'http://www.example.com/ns?p=''23''''::b
}</out>
  