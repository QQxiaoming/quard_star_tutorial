(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-7             :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator used  :)
(:as an argument to the "fn:string" function).           :)
(:*******************************************************:)
 
fn:string(xs:dateTime("1989-07-05T02:02:02Z") - xs:dateTime("1988-01-28T03:03:03Z"))