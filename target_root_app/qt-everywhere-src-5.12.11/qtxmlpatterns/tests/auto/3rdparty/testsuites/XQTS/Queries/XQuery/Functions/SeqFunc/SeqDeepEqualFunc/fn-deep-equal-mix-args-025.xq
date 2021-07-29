(:****************************************************************:) 
(: Test: fn-deep-equal-mix-args-025.xq                            :) 
(: Written By: Pulkita Tyagi                                      :) 
(: Date: Thu May 12 08:06:51 2005                                 :) 
(: Purpose: Comparing arg1:boolean arg2:value returned by true fn :) 
(:****************************************************************:) 
 
fn:deep-equal( xs:boolean("true") , true()) 
