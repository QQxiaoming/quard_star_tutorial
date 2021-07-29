(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-4                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The string value of "subtract-dates-yielding-DTD" :)
(: operator that return true and used together with fn:not. :)
(:*******************************************************:)
 
fn:not(fn:string(xs:date("1998-09-12Z") - xs:date("1998-09-21Z")))