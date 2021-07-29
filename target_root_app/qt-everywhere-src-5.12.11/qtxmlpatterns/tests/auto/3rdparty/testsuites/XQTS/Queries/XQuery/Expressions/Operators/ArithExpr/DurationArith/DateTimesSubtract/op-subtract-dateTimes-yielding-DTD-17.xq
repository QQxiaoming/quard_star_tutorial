(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-17            :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" function :)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string(xs:dateTime("2000-12-12T12:07:08Z") - xs:dateTime("1999-12-12T13:08:09Z")) and fn:false()