xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-011                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use EQName with null namespace                :)
(:*******************************************************:)

declare variable $data := <a><b>42</b></a>;

<out xmlns="http://www.example.com/ns">{
  $data / "":b
}</out>  
