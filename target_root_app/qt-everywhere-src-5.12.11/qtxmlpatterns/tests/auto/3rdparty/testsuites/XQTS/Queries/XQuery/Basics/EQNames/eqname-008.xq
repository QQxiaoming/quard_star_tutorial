xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-008                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use character references in an EQName         :)
(:*******************************************************:)


<out>{
  (<a xmlns:ex="http://www.example.com/ns"><ex:b>93.7</ex:b></a>)
     /"http://www&#x2e;example&#x2E;com/ns":b
}</out>
  