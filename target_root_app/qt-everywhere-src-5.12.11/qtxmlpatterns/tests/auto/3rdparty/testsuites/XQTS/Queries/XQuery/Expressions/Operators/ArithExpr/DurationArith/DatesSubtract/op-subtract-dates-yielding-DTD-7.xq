(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-7                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:date("1989-07-05Z") - xs:date("1962-09-04Z"))