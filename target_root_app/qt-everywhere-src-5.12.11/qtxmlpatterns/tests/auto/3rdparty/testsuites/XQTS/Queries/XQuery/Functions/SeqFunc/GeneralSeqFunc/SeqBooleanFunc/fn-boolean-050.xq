(:*******************************************************:) 
(: Test: fn-boolean-050.xq                               :) 
(: Written By: Carmelo Montanez                          :) 
(: Date: February 6, 2006                                :) 
(: Purpose: Raise error condition FORG0006 for fn:boolean.:) 
(:*******************************************************:) 
 
fn:boolean(xs:dateTime("1999-12-31T00:00:00")) 