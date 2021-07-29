xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-001                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use EQName in a path expression               :)
(:*******************************************************:)


<out>{
  (<my:a xmlns:my="http://www.example.com/ns/my"><my:b>42</my:b></my:a>) / "http://www.example.com/ns/my":b
}</out>  
