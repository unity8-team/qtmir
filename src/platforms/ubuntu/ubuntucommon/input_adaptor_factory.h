// This file is part of QtUbuntu, a set of Qt components for Ubuntu.
// Copyright © 2013 Canonical Ltd.
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 3, as published by
// the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
// SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef QUBUNTU_INPUT_ADAPTOR_FACTORY_H_
#define QUBUNTU_INPUT_ADAPTOR_FACTORY_H_

class QUbuntuInput;
class QUbuntuIntegration;

class QUbuntuInputAdaptorFactory {
 public:
  virtual ~QUbuntuInputAdaptorFactory() {}
    
  virtual QUbuntuInput* create_input_adaptor(QUbuntuIntegration* integration) = 0;

 protected:
  QUbuntuInputAdaptorFactory() {}
};

#endif
