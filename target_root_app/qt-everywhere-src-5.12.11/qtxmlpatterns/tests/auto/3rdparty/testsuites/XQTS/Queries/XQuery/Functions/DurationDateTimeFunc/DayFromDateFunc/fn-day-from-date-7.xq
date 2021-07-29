(:*******************************************************:)
(:Test: day-from-date-7                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-date" function        :)
(:used as arguments to an avg function     .             :) 
(:*******************************************************:)

fn:avg((fn:day-from-date(xs:date("1999-12-31Z")),fn:day-from-date(xs:date("1999-12-29Z")))) 