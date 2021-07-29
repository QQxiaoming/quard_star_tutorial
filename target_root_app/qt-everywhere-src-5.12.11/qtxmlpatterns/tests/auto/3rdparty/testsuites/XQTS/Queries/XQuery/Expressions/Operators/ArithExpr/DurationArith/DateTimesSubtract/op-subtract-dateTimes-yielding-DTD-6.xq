(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-6             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator that :)
(:is used as an argument to the fn:number function.      :)
(:*******************************************************:)
 
fn:number(xs:dateTime("1988-01-28T10:09:08Z") - xs:dateTime("1987-01-01T01:01:02Z"))