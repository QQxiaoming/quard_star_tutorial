(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-12                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates string value The "adjust-dateTime-to-timezone" :)
(:function as an argument to a boolean function.         :)
(:*******************************************************:)

fn:boolean(fn:string(fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00"),())))